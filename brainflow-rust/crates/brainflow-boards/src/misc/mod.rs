//! Miscellaneous board implementations.
//!
//! This module contains implementations for less common boards that don't
//! warrant their own module.

use alloc::collections::VecDeque;
use alloc::string::String;
use alloc::vec;
use alloc::vec::Vec;
use core::sync::atomic::{AtomicBool, AtomicU64, Ordering};

use brainflow_io::RingBuffer;
use brainflow_sys::error::{Error, ErrorCode, Result};

use crate::board::{reshape_to_row_major, Board, BoardDescription, BoardState};
use crate::preset::Preset;

// ============================================================================
// PIEEG (Raspberry Pi EEG HAT)
// ============================================================================

/// PIEEG board - Raspberry Pi EEG HAT.
///
/// 8-channel EEG board using ADS1299, connects via SPI to Raspberry Pi.
pub struct PieegBoard {
    state: BoardState,
    descriptions: [BoardDescription; 3],
    buffers: [Option<RingBuffer<f64>>; 3],
    marker_queue: VecDeque<f64>,
    running: AtomicBool,
    sample_count: AtomicU64,
}

impl PieegBoard {
    /// PIEEG sampling rate.
    pub const SAMPLING_RATE: u32 = 250;

    /// Number of EEG channels.
    pub const NUM_EEG_CHANNELS: usize = 8;

    /// Create a new PIEEG board.
    pub fn new() -> Self {
        Self {
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
        BoardDescription {
            sampling_rate: Self::SAMPLING_RATE,
            num_channels: 11, // package + 8 EEG + timestamp + marker
            eeg_channels: (1..=8).collect(),
            emg_channels: vec![],
            ecg_channels: vec![],
            eog_channels: vec![],
            ppg_channels: vec![],
            eda_channels: vec![],
            accel_channels: vec![],
            gyro_channels: vec![],
            temperature_channels: vec![],
            battery_channel: None,
            timestamp_channel: Some(9),
            marker_channel: Some(10),
            package_num_channel: Some(0),
            name: String::from("PIEEG"),
        }
    }

    fn create_auxiliary_description() -> BoardDescription {
        BoardDescription {
            sampling_rate: Self::SAMPLING_RATE,
            num_channels: 1,
            eeg_channels: vec![],
            emg_channels: vec![],
            ecg_channels: vec![],
            eog_channels: vec![],
            ppg_channels: vec![],
            eda_channels: vec![],
            accel_channels: vec![],
            gyro_channels: vec![],
            temperature_channels: vec![],
            battery_channel: None,
            timestamp_channel: None,
            marker_channel: None,
            package_num_channel: Some(0),
            name: String::from("PIEEG Aux"),
        }
    }

    fn create_ancillary_description() -> BoardDescription {
        BoardDescription {
            sampling_rate: Self::SAMPLING_RATE,
            num_channels: 1,
            eeg_channels: vec![],
            emg_channels: vec![],
            ecg_channels: vec![],
            eog_channels: vec![],
            ppg_channels: vec![],
            eda_channels: vec![],
            accel_channels: vec![],
            gyro_channels: vec![],
            temperature_channels: vec![],
            battery_channel: None,
            timestamp_channel: None,
            marker_channel: None,
            package_num_channel: Some(0),
            name: String::from("PIEEG Ancillary"),
        }
    }

    fn preset_index(preset: Preset) -> usize {
        match preset {
            Preset::Default => 0,
            Preset::Auxiliary => 1,
            Preset::Ancillary => 2,
        }
    }
}

impl Default for PieegBoard {
    fn default() -> Self {
        Self::new()
    }
}

impl Board for PieegBoard {
    fn state(&self) -> BoardState { self.state }
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
        if self.state == BoardState::Streaming { self.stop_stream()?; }
        self.state = BoardState::NotReady;
        self.buffers = [None, None, None];
        Ok(())
    }

    fn start_stream(&mut self, buffer_size: usize) -> Result<()> {
        if self.state != BoardState::Ready {
            return Err(Error::new(ErrorCode::BoardNotReady));
        }
        for (i, desc) in self.descriptions.iter().enumerate() {
            self.buffers[i] = Some(RingBuffer::new(buffer_size, desc.num_channels));
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
            Ok(reshape_to_row_major(&buffer.peek(num_samples), self.descriptions[idx].num_channels))
        } else {
            Err(Error::new(ErrorCode::BoardNotReady))
        }
    }

    fn get_board_data(&self, num_samples: Option<usize>, preset: Preset) -> Result<Vec<f64>> {
        let idx = Self::preset_index(preset);
        if let Some(ref buffer) = self.buffers[idx] {
            let count = buffer.len();
            let to_get = num_samples.unwrap_or(count).min(count);
            Ok(reshape_to_row_major(&buffer.pop(to_get), self.descriptions[idx].num_channels))
        } else {
            Err(Error::new(ErrorCode::BoardNotReady))
        }
    }

    fn get_board_data_count(&self, preset: Preset) -> Result<usize> {
        let idx = Self::preset_index(preset);
        self.buffers[idx].as_ref().map(|b| b.len()).ok_or_else(|| Error::new(ErrorCode::BoardNotReady))
    }

    fn insert_marker(&mut self, value: f64, _preset: Preset) -> Result<()> {
        self.marker_queue.push_back(value);
        Ok(())
    }

    fn config_board(&mut self, _config: &str) -> Result<String> { Ok(String::new()) }
}

// ============================================================================
// Unicorn (g.tec Unicorn Hybrid Black)
// ============================================================================

/// Unicorn board - g.tec Unicorn Hybrid Black.
///
/// 8-channel EEG headset. Note: Full functionality requires g.tec SDK.
pub struct UnicornBoard {
    state: BoardState,
    descriptions: [BoardDescription; 3],
    buffers: [Option<RingBuffer<f64>>; 3],
    marker_queue: VecDeque<f64>,
    running: AtomicBool,
    sample_count: AtomicU64,
}

impl UnicornBoard {
    /// Unicorn sampling rate.
    pub const SAMPLING_RATE: u32 = 250;

    /// Create a new Unicorn board.
    pub fn new() -> Self {
        Self {
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
        BoardDescription {
            sampling_rate: Self::SAMPLING_RATE,
            num_channels: 17, // package + 8 EEG + 3 accel + 3 gyro + timestamp + marker
            eeg_channels: (1..=8).collect(),
            emg_channels: vec![],
            ecg_channels: vec![],
            eog_channels: vec![],
            ppg_channels: vec![],
            eda_channels: vec![],
            accel_channels: vec![9, 10, 11],
            gyro_channels: vec![12, 13, 14],
            temperature_channels: vec![],
            battery_channel: None,
            timestamp_channel: Some(15),
            marker_channel: Some(16),
            package_num_channel: Some(0),
            name: String::from("Unicorn"),
        }
    }

    fn create_auxiliary_description() -> BoardDescription {
        BoardDescription {
            sampling_rate: Self::SAMPLING_RATE,
            num_channels: 8,
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
            timestamp_channel: Some(7),
            marker_channel: None,
            package_num_channel: Some(0),
            name: String::from("Unicorn Aux"),
        }
    }

    fn create_ancillary_description() -> BoardDescription {
        BoardDescription {
            sampling_rate: 1,
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
            name: String::from("Unicorn Ancillary"),
        }
    }

    fn preset_index(preset: Preset) -> usize {
        match preset {
            Preset::Default => 0,
            Preset::Auxiliary => 1,
            Preset::Ancillary => 2,
        }
    }
}

impl Default for UnicornBoard {
    fn default() -> Self { Self::new() }
}

impl Board for UnicornBoard {
    fn state(&self) -> BoardState { self.state }
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
        if self.state == BoardState::Streaming { self.stop_stream()?; }
        self.state = BoardState::NotReady;
        self.buffers = [None, None, None];
        Ok(())
    }

    fn start_stream(&mut self, buffer_size: usize) -> Result<()> {
        if self.state != BoardState::Ready {
            return Err(Error::new(ErrorCode::BoardNotReady));
        }
        for (i, desc) in self.descriptions.iter().enumerate() {
            self.buffers[i] = Some(RingBuffer::new(buffer_size, desc.num_channels));
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
            Ok(reshape_to_row_major(&buffer.peek(num_samples), self.descriptions[idx].num_channels))
        } else {
            Err(Error::new(ErrorCode::BoardNotReady))
        }
    }

    fn get_board_data(&self, num_samples: Option<usize>, preset: Preset) -> Result<Vec<f64>> {
        let idx = Self::preset_index(preset);
        if let Some(ref buffer) = self.buffers[idx] {
            let count = buffer.len();
            let to_get = num_samples.unwrap_or(count).min(count);
            Ok(reshape_to_row_major(&buffer.pop(to_get), self.descriptions[idx].num_channels))
        } else {
            Err(Error::new(ErrorCode::BoardNotReady))
        }
    }

    fn get_board_data_count(&self, preset: Preset) -> Result<usize> {
        let idx = Self::preset_index(preset);
        self.buffers[idx].as_ref().map(|b| b.len()).ok_or_else(|| Error::new(ErrorCode::BoardNotReady))
    }

    fn insert_marker(&mut self, value: f64, _preset: Preset) -> Result<()> {
        self.marker_queue.push_back(value);
        Ok(())
    }

    fn config_board(&mut self, _config: &str) -> Result<String> { Ok(String::new()) }
}

// ============================================================================
// Callibri (NeuroMD Callibri)
// ============================================================================

/// Callibri variant.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum CallibriVariant {
    /// EEG mode.
    Eeg,
    /// EMG mode.
    Emg,
    /// ECG mode.
    Ecg,
}

/// Callibri board - NeuroMD Callibri sensor.
///
/// Multi-purpose biosignal sensor. Note: Full functionality requires NeuroMD SDK.
pub struct CallibriBoard {
    variant: CallibriVariant,
    state: BoardState,
    descriptions: [BoardDescription; 3],
    buffers: [Option<RingBuffer<f64>>; 3],
    marker_queue: VecDeque<f64>,
    running: AtomicBool,
    sample_count: AtomicU64,
}

impl CallibriBoard {
    /// Callibri sampling rate.
    pub const SAMPLING_RATE: u32 = 250;

    /// Create a new Callibri board.
    pub fn new(variant: CallibriVariant) -> Self {
        Self {
            variant,
            state: BoardState::NotReady,
            descriptions: [
                Self::create_default_description(variant),
                Self::create_auxiliary_description(),
                Self::create_ancillary_description(),
            ],
            buffers: [None, None, None],
            marker_queue: VecDeque::new(),
            running: AtomicBool::new(false),
            sample_count: AtomicU64::new(0),
        }
    }

    fn create_default_description(variant: CallibriVariant) -> BoardDescription {
        let (name, eeg, emg, ecg) = match variant {
            CallibriVariant::Eeg => ("Callibri EEG", vec![1], vec![], vec![]),
            CallibriVariant::Emg => ("Callibri EMG", vec![], vec![1], vec![]),
            CallibriVariant::Ecg => ("Callibri ECG", vec![], vec![], vec![1]),
        };

        BoardDescription {
            sampling_rate: Self::SAMPLING_RATE,
            num_channels: 4, // package + signal + timestamp + marker
            eeg_channels: eeg,
            emg_channels: emg,
            ecg_channels: ecg,
            eog_channels: vec![],
            ppg_channels: vec![],
            eda_channels: vec![],
            accel_channels: vec![],
            gyro_channels: vec![],
            temperature_channels: vec![],
            battery_channel: None,
            timestamp_channel: Some(2),
            marker_channel: Some(3),
            package_num_channel: Some(0),
            name: String::from(name),
        }
    }

    fn create_auxiliary_description() -> BoardDescription {
        BoardDescription {
            sampling_rate: 50,
            num_channels: 5,
            eeg_channels: vec![],
            emg_channels: vec![],
            ecg_channels: vec![],
            eog_channels: vec![],
            ppg_channels: vec![],
            eda_channels: vec![],
            accel_channels: vec![1, 2, 3],
            gyro_channels: vec![],
            temperature_channels: vec![],
            battery_channel: None,
            timestamp_channel: Some(4),
            marker_channel: None,
            package_num_channel: Some(0),
            name: String::from("Callibri Aux"),
        }
    }

    fn create_ancillary_description() -> BoardDescription {
        BoardDescription {
            sampling_rate: 1,
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
            name: String::from("Callibri Ancillary"),
        }
    }

    fn preset_index(preset: Preset) -> usize {
        match preset {
            Preset::Default => 0,
            Preset::Auxiliary => 1,
            Preset::Ancillary => 2,
        }
    }

    /// Get the variant.
    pub fn variant(&self) -> CallibriVariant { self.variant }
}

impl Board for CallibriBoard {
    fn state(&self) -> BoardState { self.state }
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
        if self.state == BoardState::Streaming { self.stop_stream()?; }
        self.state = BoardState::NotReady;
        self.buffers = [None, None, None];
        Ok(())
    }

    fn start_stream(&mut self, buffer_size: usize) -> Result<()> {
        if self.state != BoardState::Ready {
            return Err(Error::new(ErrorCode::BoardNotReady));
        }
        for (i, desc) in self.descriptions.iter().enumerate() {
            self.buffers[i] = Some(RingBuffer::new(buffer_size, desc.num_channels));
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
            Ok(reshape_to_row_major(&buffer.peek(num_samples), self.descriptions[idx].num_channels))
        } else {
            Err(Error::new(ErrorCode::BoardNotReady))
        }
    }

    fn get_board_data(&self, num_samples: Option<usize>, preset: Preset) -> Result<Vec<f64>> {
        let idx = Self::preset_index(preset);
        if let Some(ref buffer) = self.buffers[idx] {
            let count = buffer.len();
            let to_get = num_samples.unwrap_or(count).min(count);
            Ok(reshape_to_row_major(&buffer.pop(to_get), self.descriptions[idx].num_channels))
        } else {
            Err(Error::new(ErrorCode::BoardNotReady))
        }
    }

    fn get_board_data_count(&self, preset: Preset) -> Result<usize> {
        let idx = Self::preset_index(preset);
        self.buffers[idx].as_ref().map(|b| b.len()).ok_or_else(|| Error::new(ErrorCode::BoardNotReady))
    }

    fn insert_marker(&mut self, value: f64, _preset: Preset) -> Result<()> {
        self.marker_queue.push_back(value);
        Ok(())
    }

    fn config_board(&mut self, _config: &str) -> Result<String> { Ok(String::new()) }
}

// ============================================================================
// Simple stub boards for remaining types
// ============================================================================

macro_rules! simple_board {
    ($name:ident, $display:expr, $rate:expr, $eeg:expr) => {
        /// Simple board implementation.
        pub struct $name {
            state: BoardState,
            description: BoardDescription,
            buffer: Option<RingBuffer<f64>>,
            running: AtomicBool,
        }

        impl $name {
            /// Create a new board.
            pub fn new() -> Self {
                let num_channels = 1 + $eeg + 2; // package + eeg + timestamp + marker
                Self {
                    state: BoardState::NotReady,
                    description: BoardDescription {
                        sampling_rate: $rate,
                        num_channels,
                        eeg_channels: (1..=$eeg).collect(),
                        emg_channels: vec![],
                        ecg_channels: vec![],
                        eog_channels: vec![],
                        ppg_channels: vec![],
                        eda_channels: vec![],
                        accel_channels: vec![],
                        gyro_channels: vec![],
                        temperature_channels: vec![],
                        battery_channel: None,
                        timestamp_channel: Some($eeg + 1),
                        marker_channel: Some($eeg + 2),
                        package_num_channel: Some(0),
                        name: String::from($display),
                    },
                    buffer: None,
                    running: AtomicBool::new(false),
                }
            }
        }

        impl Default for $name {
            fn default() -> Self { Self::new() }
        }

        impl Board for $name {
            fn state(&self) -> BoardState { self.state }
            fn description(&self, _preset: Preset) -> &BoardDescription { &self.description }

            fn prepare_session(&mut self) -> Result<()> {
                if self.state != BoardState::NotReady {
                    return Err(Error::new(ErrorCode::PortAlreadyOpen));
                }
                self.state = BoardState::Ready;
                Ok(())
            }

            fn release_session(&mut self) -> Result<()> {
                if self.state == BoardState::Streaming { self.stop_stream()?; }
                self.state = BoardState::NotReady;
                self.buffer = None;
                Ok(())
            }

            fn start_stream(&mut self, buffer_size: usize) -> Result<()> {
                if self.state != BoardState::Ready {
                    return Err(Error::new(ErrorCode::BoardNotReady));
                }
                self.buffer = Some(RingBuffer::new(buffer_size, self.description.num_channels));
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

            fn get_current_board_data(&self, num_samples: usize, _preset: Preset) -> Result<Vec<f64>> {
                if let Some(ref buffer) = self.buffer {
                    Ok(reshape_to_row_major(&buffer.peek(num_samples), self.description.num_channels))
                } else {
                    Err(Error::new(ErrorCode::BoardNotReady))
                }
            }

            fn get_board_data(&self, num_samples: Option<usize>, _preset: Preset) -> Result<Vec<f64>> {
                if let Some(ref buffer) = self.buffer {
                    let count = buffer.len();
                    let to_get = num_samples.unwrap_or(count).min(count);
                    Ok(reshape_to_row_major(&buffer.pop(to_get), self.description.num_channels))
                } else {
                    Err(Error::new(ErrorCode::BoardNotReady))
                }
            }

            fn get_board_data_count(&self, _preset: Preset) -> Result<usize> {
                self.buffer.as_ref().map(|b| b.len()).ok_or_else(|| Error::new(ErrorCode::BoardNotReady))
            }

            fn insert_marker(&mut self, _value: f64, _preset: Preset) -> Result<()> { Ok(()) }
            fn config_board(&mut self, _config: &str) -> Result<String> { Ok(String::new()) }
        }
    };
}

// Generate simple boards
simple_board!(IronbciBoard, "Ironbci", 250, 8);
simple_board!(FasciaBoard, "Fascia", 500, 16);
simple_board!(EnophoneBoard, "Enophone", 250, 4);
simple_board!(BrainAliveBoard, "BrainAlive", 250, 8);
simple_board!(GanglionNativeBoard, "Ganglion Native", 200, 4);
simple_board!(NtlWifiBoard, "NTL WiFi", 250, 8);
simple_board!(AavaaV3Board, "AAVAA V3", 256, 2);
simple_board!(NeuropawnKnightBoard, "Neuropawn Knight", 500, 8);
simple_board!(SynchroniBoard, "Synchroni", 250, 8);

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_pieeg() {
        let board = PieegBoard::new();
        assert_eq!(board.description(Preset::Default).eeg_channels.len(), 8);
    }

    #[test]
    fn test_unicorn() {
        let board = UnicornBoard::new();
        assert_eq!(board.description(Preset::Default).eeg_channels.len(), 8);
        assert_eq!(board.description(Preset::Default).accel_channels.len(), 3);
    }

    #[test]
    fn test_callibri() {
        let board = CallibriBoard::new(CallibriVariant::Eeg);
        assert_eq!(board.description(Preset::Default).eeg_channels.len(), 1);

        let board = CallibriBoard::new(CallibriVariant::Emg);
        assert_eq!(board.description(Preset::Default).emg_channels.len(), 1);
    }

    #[test]
    fn test_simple_boards() {
        let board = IronbciBoard::new();
        assert_eq!(board.description(Preset::Default).sampling_rate, 250);

        let board = FasciaBoard::new();
        assert_eq!(board.description(Preset::Default).eeg_channels.len(), 16);
    }
}
