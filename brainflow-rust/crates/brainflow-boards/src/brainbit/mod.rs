//! BrainBit board implementations.
//!
//! BrainBit is a wireless EEG headband that streams data over BLE.
//! Provides 4 EEG channels at 250 Hz.

use alloc::collections::VecDeque;
use alloc::string::String;
use alloc::vec;
use alloc::vec::Vec;
use core::sync::atomic::{AtomicBool, AtomicU64, Ordering};

use brainflow_io::RingBuffer;
use brainflow_sys::error::{Error, ErrorCode, Result};

use crate::board::{reshape_to_row_major, Board, BoardDescription, BoardState};
use crate::preset::Preset;

/// BrainBit variant.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum BrainBitVariant {
    /// Standard BrainBit (native BLE).
    Standard,
    /// BrainBit BLED (SimpleBLE).
    Bled,
}

/// BrainBit EEG headband.
///
/// 4-channel EEG device with dry electrodes.
/// Uses BLE for data streaming at 250 Hz.
pub struct BrainBitBoard {
    variant: BrainBitVariant,
    state: BoardState,
    descriptions: [BoardDescription; 3],
    buffers: [Option<RingBuffer<f64>>; 3],
    marker_queue: VecDeque<f64>,
    running: AtomicBool,
    sample_count: AtomicU64,
}

impl BrainBitBoard {
    /// BrainBit sampling rate.
    pub const SAMPLING_RATE: u32 = 250;

    /// Number of EEG channels.
    pub const NUM_EEG_CHANNELS: usize = 4;

    /// EEG scale factor (24-bit ADC).
    pub const EEG_SCALE: f64 = 1.0 / 8388607.0 * 1_000_000.0;

    /// Create a new BrainBit board.
    pub fn new(variant: BrainBitVariant) -> Self {
        Self {
            variant,
            state: BoardState::NotReady,
            descriptions: [
                Self::create_default_description(),
                Self::create_auxiliary_description(),
                Self::create_ancillary_description(),
            ],
            buffers: [None, None, None],
            marker_queue: VecDeque::new(),
            running: AtomicBool::new(false),
            sample_count: AtomicU64::new(0),
        }
    }

    fn create_default_description() -> BoardDescription {
        // Default preset: EEG channels
        // Channels: package, T3, T4, O1, O2, timestamp, marker
        let num_channels = 7;

        BoardDescription {
            sampling_rate: Self::SAMPLING_RATE,
            num_channels,
            eeg_channels: vec![1, 2, 3, 4], // T3, T4, O1, O2
            emg_channels: vec![],
            ecg_channels: vec![],
            eog_channels: vec![],
            ppg_channels: vec![],
            eda_channels: vec![],
            accel_channels: vec![],
            gyro_channels: vec![],
            temperature_channels: vec![],
            battery_channel: None,
            timestamp_channel: Some(5),
            marker_channel: Some(6),
            package_num_channel: Some(0),
            name: String::from("BrainBit"),
        }
    }

    fn create_auxiliary_description() -> BoardDescription {
        // Auxiliary preset: Resistance data
        BoardDescription {
            sampling_rate: Self::SAMPLING_RATE,
            num_channels: 6,
            eeg_channels: vec![],
            emg_channels: vec![],
            ecg_channels: vec![],
            eog_channels: vec![],
            ppg_channels: vec![],
            eda_channels: vec![1, 2, 3, 4], // Resistance values for 4 channels
            accel_channels: vec![],
            gyro_channels: vec![],
            temperature_channels: vec![],
            battery_channel: None,
            timestamp_channel: Some(5),
            marker_channel: None,
            package_num_channel: Some(0),
            name: String::from("BrainBit Aux"),
        }
    }

    fn create_ancillary_description() -> BoardDescription {
        // Ancillary preset: Battery and status
        BoardDescription {
            sampling_rate: Self::SAMPLING_RATE,
            num_channels: 3,
            eeg_channels: vec![],
            emg_channels: vec![],
            ecg_channels: vec![],
            eog_channels: vec![],
            ppg_channels: vec![],
            eda_channels: vec![],
            accel_channels: vec![],
            gyro_channels: vec![],
            temperature_channels: vec![],
            battery_channel: Some(1),
            timestamp_channel: Some(2),
            marker_channel: None,
            package_num_channel: Some(0),
            name: String::from("BrainBit Ancillary"),
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
    pub fn variant(&self) -> BrainBitVariant {
        self.variant
    }
}

impl Board for BrainBitBoard {
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
        // 1. Scan for BrainBit devices via BLE
        // 2. Connect to device
        // 3. Subscribe to EEG characteristic

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
    fn test_brainbit_creation() {
        let board = BrainBitBoard::new(BrainBitVariant::Standard);
        assert_eq!(board.state(), BoardState::NotReady);
        assert_eq!(board.variant(), BrainBitVariant::Standard);

        let desc = board.description(Preset::Default);
        assert_eq!(desc.sampling_rate, 250);
        assert_eq!(desc.eeg_channels.len(), 4);
    }

    #[test]
    fn test_brainbit_lifecycle() {
        let mut board = BrainBitBoard::new(BrainBitVariant::Bled);

        board.prepare_session().unwrap();
        assert_eq!(board.state(), BoardState::Ready);

        board.start_stream(1000).unwrap();
        assert_eq!(board.state(), BoardState::Streaming);

        board.stop_stream().unwrap();
        assert_eq!(board.state(), BoardState::Ready);

        board.release_session().unwrap();
        assert_eq!(board.state(), BoardState::NotReady);
    }
}
