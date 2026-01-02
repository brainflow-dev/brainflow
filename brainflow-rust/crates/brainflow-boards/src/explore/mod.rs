//! Mentalab Explore board implementations.
//!
//! Explore is a wireless EEG/ExG device that streams data over BLE.
//! Available in 4, 8, and 32 channel variants.

use alloc::collections::VecDeque;
use alloc::string::String;
use alloc::vec;
use alloc::vec::Vec;
use core::sync::atomic::{AtomicBool, AtomicU64, Ordering};

use brainflow_io::RingBuffer;
use brainflow_sys::error::{Error, ErrorCode, Result};

use crate::board::{reshape_to_row_major, Board, BoardDescription, BoardState};
use crate::preset::Preset;

/// Explore variant (channel count).
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ExploreVariant {
    /// 4 channel Explore.
    Chan4,
    /// 8 channel Explore.
    Chan8,
    /// 32 channel Explore.
    Chan32,
    /// 8 channel Explore Plus.
    Plus8,
    /// 32 channel Explore Plus.
    Plus32,
}

impl ExploreVariant {
    /// Get the number of EEG channels for this variant.
    pub fn num_channels(&self) -> usize {
        match self {
            ExploreVariant::Chan4 => 4,
            ExploreVariant::Chan8 | ExploreVariant::Plus8 => 8,
            ExploreVariant::Chan32 | ExploreVariant::Plus32 => 32,
        }
    }

    /// Get the sampling rate for this variant.
    pub fn sampling_rate(&self) -> u32 {
        match self {
            ExploreVariant::Chan4 | ExploreVariant::Chan8 | ExploreVariant::Chan32 => 250,
            ExploreVariant::Plus8 | ExploreVariant::Plus32 => 500,
        }
    }

    /// Check if this is a Plus variant.
    pub fn is_plus(&self) -> bool {
        matches!(self, ExploreVariant::Plus8 | ExploreVariant::Plus32)
    }
}

/// Mentalab Explore EEG device.
///
/// Wireless EEG device with BLE streaming.
pub struct ExploreBoard {
    variant: ExploreVariant,
    state: BoardState,
    descriptions: [BoardDescription; 3],
    buffers: [Option<RingBuffer<f64>>; 3],
    marker_queue: VecDeque<f64>,
    running: AtomicBool,
    sample_count: AtomicU64,
}

impl ExploreBoard {
    /// EEG scale factor.
    pub const EEG_SCALE: f64 = 1.0 / 8388607.0 * 1_000_000.0;

    /// Create a new Explore board.
    pub fn new(variant: ExploreVariant) -> Self {
        let descriptions = [
            Self::create_default_description(variant),
            Self::create_auxiliary_description(variant),
            Self::create_ancillary_description(variant),
        ];

        Self {
            variant,
            state: BoardState::NotReady,
            descriptions,
            buffers: [None, None, None],
            marker_queue: VecDeque::new(),
            running: AtomicBool::new(false),
            sample_count: AtomicU64::new(0),
        }
    }

    fn create_default_description(variant: ExploreVariant) -> BoardDescription {
        let num_eeg = variant.num_channels();
        // Channels: package, EEG..., timestamp, marker
        let num_channels = 1 + num_eeg + 2;

        BoardDescription {
            sampling_rate: variant.sampling_rate(),
            num_channels,
            eeg_channels: (1..=num_eeg).collect(),
            emg_channels: vec![],
            ecg_channels: vec![],
            eog_channels: vec![],
            ppg_channels: vec![],
            eda_channels: vec![],
            accel_channels: vec![],
            gyro_channels: vec![],
            temperature_channels: vec![],
            battery_channel: None,
            timestamp_channel: Some(num_eeg + 1),
            marker_channel: Some(num_eeg + 2),
            package_num_channel: Some(0),
            name: String::from(match variant {
                ExploreVariant::Chan4 => "Explore 4ch",
                ExploreVariant::Chan8 => "Explore 8ch",
                ExploreVariant::Chan32 => "Explore 32ch",
                ExploreVariant::Plus8 => "Explore+ 8ch",
                ExploreVariant::Plus32 => "Explore+ 32ch",
            }),
        }
    }

    fn create_auxiliary_description(variant: ExploreVariant) -> BoardDescription {
        // Auxiliary preset: IMU data
        // Channels: package, accel_x, accel_y, accel_z, gyro_x, gyro_y, gyro_z, mag_x, mag_y, mag_z, timestamp
        BoardDescription {
            sampling_rate: if variant.is_plus() { 100 } else { 20 },
            num_channels: 11,
            eeg_channels: vec![],
            emg_channels: vec![],
            ecg_channels: vec![],
            eog_channels: vec![],
            ppg_channels: vec![],
            eda_channels: vec![],
            accel_channels: vec![1, 2, 3],
            gyro_channels: vec![4, 5, 6],
            temperature_channels: vec![],
            battery_channel: None,
            timestamp_channel: Some(10),
            marker_channel: None,
            package_num_channel: Some(0),
            name: String::from("Explore Aux"),
        }
    }

    fn create_ancillary_description(variant: ExploreVariant) -> BoardDescription {
        // Ancillary preset: Environmental sensors
        BoardDescription {
            sampling_rate: 1,
            num_channels: 5,
            eeg_channels: vec![],
            emg_channels: vec![],
            ecg_channels: vec![],
            eog_channels: vec![],
            ppg_channels: vec![],
            eda_channels: vec![],
            accel_channels: vec![],
            gyro_channels: vec![],
            temperature_channels: vec![1],
            battery_channel: Some(2),
            timestamp_channel: Some(4),
            marker_channel: None,
            package_num_channel: Some(0),
            name: String::from("Explore Ancillary"),
        }
    }

    fn preset_index(preset: Preset) -> usize {
        match preset {
            Preset::Default => 0,
            Preset::Auxiliary => 1,
            Preset::Ancillary => 2,
        }
    }

    /// Get the variant type.
    pub fn variant(&self) -> ExploreVariant {
        self.variant
    }
}

impl Board for ExploreBoard {
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

        // In a real implementation:
        // 1. Scan for Explore devices via BLE
        // 2. Connect to device
        // 3. Configure channels and sampling rate

        self.state = BoardState::Ready;
        Ok(())
    }

    fn release_session(&mut self) -> Result<()> {
        if self.state == BoardState::Streaming {
            self.stop_stream()?;
        }

        self.state = BoardState::NotReady;
        self.buffers = [None, None, None];
        self.marker_queue.clear();
        self.sample_count.store(0, Ordering::Relaxed);

        Ok(())
    }

    fn start_stream(&mut self, buffer_size: usize) -> Result<()> {
        if self.state != BoardState::Ready {
            return Err(Error::new(ErrorCode::BoardNotReady));
        }

        for (i, desc) in self.descriptions.iter().enumerate() {
            if desc.num_channels > 0 {
                self.buffers[i] = Some(RingBuffer::new(buffer_size, desc.num_channels));
            }
        }

        self.running.store(true, Ordering::Release);
        self.state = BoardState::Streaming;

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

    fn insert_marker(&mut self, value: f64, _preset: Preset) -> Result<()> {
        self.marker_queue.push_back(value);
        Ok(())
    }

    fn config_board(&mut self, _config: &str) -> Result<String> {
        Ok(String::new())
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_explore_variants() {
        assert_eq!(ExploreVariant::Chan4.num_channels(), 4);
        assert_eq!(ExploreVariant::Chan8.num_channels(), 8);
        assert_eq!(ExploreVariant::Chan32.num_channels(), 32);
        assert_eq!(ExploreVariant::Plus8.num_channels(), 8);
        assert_eq!(ExploreVariant::Plus32.num_channels(), 32);

        assert_eq!(ExploreVariant::Chan8.sampling_rate(), 250);
        assert_eq!(ExploreVariant::Plus8.sampling_rate(), 500);
    }

    #[test]
    fn test_explore_creation() {
        let board = ExploreBoard::new(ExploreVariant::Chan8);
        assert_eq!(board.state(), BoardState::NotReady);

        let desc = board.description(Preset::Default);
        assert_eq!(desc.sampling_rate, 250);
        assert_eq!(desc.eeg_channels.len(), 8);
    }

    #[test]
    fn test_explore_lifecycle() {
        let mut board = ExploreBoard::new(ExploreVariant::Plus32);

        board.prepare_session().unwrap();
        assert_eq!(board.state(), BoardState::Ready);

        board.start_stream(1000).unwrap();
        assert_eq!(board.state(), BoardState::Streaming);

        board.stop_stream().unwrap();
        board.release_session().unwrap();
        assert_eq!(board.state(), BoardState::NotReady);
    }
}
