//! Synthetic board implementation for testing.

use alloc::collections::VecDeque;
use alloc::string::String;
use alloc::vec::Vec;
use core::sync::atomic::{AtomicBool, AtomicU64, Ordering};

use brainflow_io::RingBuffer;
use brainflow_sys::error::{Error, ErrorCode, Result};

use crate::board::{Board, BoardDescription, BoardState, reshape_to_row_major};
use crate::preset::Preset;

/// Synthetic board that generates test data.
///
/// This matches the C++ SyntheticBoard implementation.
pub struct SyntheticBoard {
    state: BoardState,
    descriptions: [BoardDescription; 3],
    buffers: [Option<RingBuffer<f64>>; 3],
    marker_queues: [VecDeque<f64>; 3],
    package_counter: AtomicU64,
    running: AtomicBool,
    phase: Vec<f64>,
}

impl SyntheticBoard {
    /// Default sampling rate for synthetic board.
    pub const SAMPLING_RATE: u32 = 250;
    /// Number of EEG channels.
    pub const NUM_EEG_CHANNELS: usize = 8;
    /// Number of auxiliary channels.
    pub const NUM_AUX_CHANNELS: usize = 3;

    /// Create a new synthetic board.
    pub fn new() -> Self {
        let default_desc = Self::create_default_description();
        let aux_desc = Self::create_auxiliary_description();
        let ancillary_desc = BoardDescription::default();

        Self {
            state: BoardState::NotReady,
            descriptions: [default_desc, aux_desc, ancillary_desc],
            buffers: [None, None, None],
            marker_queues: [VecDeque::new(), VecDeque::new(), VecDeque::new()],
            package_counter: AtomicU64::new(0),
            running: AtomicBool::new(false),
            phase: alloc::vec![0.0; Self::NUM_EEG_CHANNELS],
        }
    }

    fn create_default_description() -> BoardDescription {
        let num_channels = Self::NUM_EEG_CHANNELS + 4; // EEG + package + timestamp + marker + battery

        BoardDescription {
            sampling_rate: Self::SAMPLING_RATE,
            num_channels,
            eeg_channels: (1..=Self::NUM_EEG_CHANNELS).collect(),
            emg_channels: Vec::new(),
            ecg_channels: Vec::new(),
            eog_channels: Vec::new(),
            ppg_channels: Vec::new(),
            eda_channels: Vec::new(),
            accel_channels: Vec::new(),
            gyro_channels: Vec::new(),
            temperature_channels: Vec::new(),
            battery_channel: Some(num_channels - 1),
            timestamp_channel: Some(num_channels - 3),
            marker_channel: Some(num_channels - 2),
            package_num_channel: Some(0),
            name: String::from("Synthetic"),
        }
    }

    fn create_auxiliary_description() -> BoardDescription {
        let num_channels = 7; // accel(3) + gyro(3) + package

        BoardDescription {
            sampling_rate: Self::SAMPLING_RATE,
            num_channels,
            eeg_channels: Vec::new(),
            emg_channels: Vec::new(),
            ecg_channels: Vec::new(),
            eog_channels: Vec::new(),
            ppg_channels: Vec::new(),
            eda_channels: Vec::new(),
            accel_channels: alloc::vec![1, 2, 3],
            gyro_channels: alloc::vec![4, 5, 6],
            temperature_channels: Vec::new(),
            battery_channel: None,
            timestamp_channel: None,
            marker_channel: None,
            package_num_channel: Some(0),
            name: String::from("Synthetic Aux"),
        }
    }

    fn preset_index(preset: Preset) -> usize {
        preset.to_raw() as usize
    }

    /// Generate a single sample of synthetic data.
    fn generate_sample(&mut self, preset: Preset) -> Vec<f64> {
        let pkg_num = self.package_counter.fetch_add(1, Ordering::Relaxed) as f64;

        match preset {
            Preset::Default => self.generate_default_sample(pkg_num),
            Preset::Auxiliary => self.generate_auxiliary_sample(pkg_num),
            Preset::Ancillary => Vec::new(),
        }
    }

    fn generate_default_sample(&mut self, pkg_num: f64) -> Vec<f64> {
        let desc = &self.descriptions[0];
        let mut sample = alloc::vec![0.0; desc.num_channels];

        // Package number
        sample[0] = pkg_num;

        // EEG channels: sine waves at different frequencies
        for (i, &ch) in desc.eeg_channels.iter().enumerate() {
            let freq = 5.0 * (i + 1) as f64; // 5, 10, 15, 20, ... Hz
            let amplitude = 10.0 * (i + 1) as f64; // μV

            // Update phase
            let dt = 1.0 / Self::SAMPLING_RATE as f64;
            self.phase[i] += 2.0 * core::f64::consts::PI * freq * dt;
            if self.phase[i] > 2.0 * core::f64::consts::PI {
                self.phase[i] -= 2.0 * core::f64::consts::PI;
            }

            // Generate sine wave with noise
            let noise = simple_random(pkg_num as u64 + i as u64) * 0.1 * amplitude;
            sample[ch] = amplitude * libm::sin(self.phase[i]) + noise;
        }

        // Timestamp (simulated)
        if let Some(ts_ch) = desc.timestamp_channel {
            sample[ts_ch] = pkg_num / Self::SAMPLING_RATE as f64;
        }

        // Marker from queue
        if let Some(marker_ch) = desc.marker_channel {
            let idx = Self::preset_index(Preset::Default);
            sample[marker_ch] = self.marker_queues[idx].pop_front().unwrap_or(0.0);
        }

        // Battery level (simulated at ~80%)
        if let Some(batt_ch) = desc.battery_channel {
            sample[batt_ch] = 80.0 + simple_random(pkg_num as u64) * 5.0;
        }

        sample
    }

    fn generate_auxiliary_sample(&self, pkg_num: f64) -> Vec<f64> {
        let desc = &self.descriptions[1];
        let mut sample = alloc::vec![0.0; desc.num_channels];

        // Package number
        sample[0] = pkg_num;

        // Accelerometer (simulate gravity on Z axis)
        for (i, &ch) in desc.accel_channels.iter().enumerate() {
            let base = if i == 2 { 1.0 } else { 0.0 }; // Gravity on Z
            sample[ch] = base + simple_random(pkg_num as u64 + 100 + i as u64) * 0.1;
        }

        // Gyroscope (small random rotation)
        for (i, &ch) in desc.gyro_channels.iter().enumerate() {
            sample[ch] = simple_random(pkg_num as u64 + 200 + i as u64) * 0.01;
        }

        sample
    }

    /// Generate samples in a loop (would be called from a thread).
    pub fn generate_samples(&mut self, num_samples: usize) {
        for preset in [Preset::Default, Preset::Auxiliary] {
            let idx = Self::preset_index(preset);
            // Check if buffer exists first
            if self.buffers[idx].is_none() {
                continue;
            }

            // Generate all samples first to avoid borrow conflicts
            let samples: Vec<Vec<f64>> = (0..num_samples)
                .map(|_| self.generate_sample(preset))
                .collect();

            // Then push them to the buffer
            if let Some(ref buffer) = self.buffers[idx] {
                for sample in samples {
                    buffer.push(&sample);
                }
            }
        }
    }
}

impl Default for SyntheticBoard {
    fn default() -> Self {
        Self::new()
    }
}

impl Board for SyntheticBoard {
    fn state(&self) -> BoardState {
        self.state
    }

    fn description(&self, preset: Preset) -> &BoardDescription {
        &self.descriptions[Self::preset_index(preset)]
    }

    fn prepare_session(&mut self) -> Result<()> {
        if self.state != BoardState::NotReady {
            return Err(Error::new(ErrorCode::PortAlreadyOpen));
        }

        self.state = BoardState::Ready;
        Ok(())
    }

    fn release_session(&mut self) -> Result<()> {
        if self.state == BoardState::Streaming {
            self.stop_stream()?;
        }

        self.state = BoardState::NotReady;
        self.buffers = [None, None, None];
        self.marker_queues = [VecDeque::new(), VecDeque::new(), VecDeque::new()];
        Ok(())
    }

    fn start_stream(&mut self, buffer_size: usize) -> Result<()> {
        if self.state != BoardState::Ready {
            return Err(Error::new(ErrorCode::BoardNotReady));
        }

        // Create buffers for each preset
        for (i, desc) in self.descriptions.iter().enumerate() {
            if desc.num_channels > 0 {
                self.buffers[i] = Some(RingBuffer::new(buffer_size, desc.num_channels));
            }
        }

        self.running.store(true, Ordering::Release);
        self.state = BoardState::Streaming;

        // In a real implementation, this would spawn a thread
        // For now, generate some initial samples
        self.generate_samples(100);

        Ok(())
    }

    fn stop_stream(&mut self) -> Result<()> {
        if self.state != BoardState::Streaming {
            return Err(Error::new(ErrorCode::StreamThreadNotRunning));
        }

        self.running.store(false, Ordering::Release);
        self.state = BoardState::Ready;
        Ok(())
    }

    fn get_current_board_data(&self, num_samples: usize, preset: Preset) -> Result<Vec<f64>> {
        let idx = Self::preset_index(preset);

        if let Some(ref buffer) = self.buffers[idx] {
            let data = buffer.peek(num_samples);
            let num_channels = self.descriptions[idx].num_channels;
            Ok(reshape_to_row_major(&data, num_channels))
        } else {
            Err(Error::new(ErrorCode::BoardNotReady))
        }
    }

    fn get_board_data(&self, num_samples: Option<usize>, preset: Preset) -> Result<Vec<f64>> {
        let idx = Self::preset_index(preset);

        if let Some(ref buffer) = self.buffers[idx] {
            let count = buffer.len();
            let to_get = num_samples.unwrap_or(count).min(count);
            let data = buffer.pop(to_get);
            let num_channels = self.descriptions[idx].num_channels;
            Ok(reshape_to_row_major(&data, num_channels))
        } else {
            Err(Error::new(ErrorCode::BoardNotReady))
        }
    }

    fn get_board_data_count(&self, preset: Preset) -> Result<usize> {
        let idx = Self::preset_index(preset);

        if let Some(ref buffer) = self.buffers[idx] {
            Ok(buffer.len())
        } else {
            Err(Error::new(ErrorCode::BoardNotReady))
        }
    }

    fn insert_marker(&mut self, value: f64, preset: Preset) -> Result<()> {
        let idx = Self::preset_index(preset);
        self.marker_queues[idx].push_back(value);
        Ok(())
    }

    fn config_board(&mut self, _config: &str) -> Result<String> {
        // Synthetic board ignores config commands
        Ok(String::new())
    }
}

/// Simple deterministic pseudo-random number generator.
fn simple_random(seed: u64) -> f64 {
    let x = seed.wrapping_mul(1103515245).wrapping_add(12345);
    ((x >> 16) & 0x7FFF) as f64 / 32768.0 - 0.5
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_synthetic_board_lifecycle() {
        let mut board = SyntheticBoard::new();

        assert_eq!(board.state(), BoardState::NotReady);

        board.prepare_session().unwrap();
        assert_eq!(board.state(), BoardState::Ready);

        board.start_stream(1000).unwrap();
        assert_eq!(board.state(), BoardState::Streaming);

        // Generate more samples
        board.generate_samples(50);

        let count = board.get_board_data_count(Preset::Default).unwrap();
        assert!(count > 0);

        let data = board.get_board_data(Some(10), Preset::Default).unwrap();
        let num_channels = board.description(Preset::Default).num_channels;
        assert_eq!(data.len(), 10 * num_channels);

        board.stop_stream().unwrap();
        assert_eq!(board.state(), BoardState::Ready);

        board.release_session().unwrap();
        assert_eq!(board.state(), BoardState::NotReady);
    }

    #[test]
    fn test_marker_insertion() {
        let mut board = SyntheticBoard::new();
        board.prepare_session().unwrap();
        board.start_stream(1000).unwrap();

        // Drain initial samples generated by start_stream
        let _ = board.get_board_data(None, Preset::Default);

        board.insert_marker(42.0, Preset::Default).unwrap();
        board.generate_samples(1);

        // The marker should appear in the next sample
        let data = board.get_board_data(Some(1), Preset::Default).unwrap();
        let desc = board.description(Preset::Default);

        if let Some(marker_ch) = desc.marker_channel {
            // Data is in row-major format: all samples of ch0, all samples of ch1, ...
            assert!((data[marker_ch] - 42.0).abs() < 1e-10);
        }

        board.release_session().unwrap();
    }
}
