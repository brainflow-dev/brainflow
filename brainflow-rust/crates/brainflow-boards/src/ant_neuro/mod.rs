//! ANT Neuro board implementations.
//!
//! Supports ANT Neuro eego amplifiers with various channel configurations.
//! These are high-quality research-grade EEG amplifiers.

use alloc::boxed::Box;
use alloc::collections::VecDeque;
use alloc::string::String;
use alloc::vec;
use alloc::vec::Vec;
use core::sync::atomic::{AtomicBool, Ordering};

use brainflow_io::RingBuffer;
use brainflow_sys::error::{Error, ErrorCode, Result};

use crate::board::{reshape_to_row_major, Board, BoardDescription, BoardState};
use crate::preset::Preset;

/// ANT Neuro amplifier variant.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum AntNeuroVariant {
    /// eego EE-410 (64 channels).
    Ee410,
    /// eego EE-411 (32 channels).
    Ee411,
    /// eego EE-430 (128 channels).
    Ee430,
    /// eego EE-211 (8 channels).
    Ee211,
    /// eego EE-212 (16 channels).
    Ee212,
    /// eego EE-213 (24 channels).
    Ee213,
    /// eego EE-214 (32 channels).
    Ee214,
    /// eego EE-215 (64 channels).
    Ee215,
    /// eego EE-221 (8 channels).
    Ee221,
    /// eego EE-222 (16 channels).
    Ee222,
    /// eego EE-223 (24 channels).
    Ee223,
    /// eego EE-224 (32 channels).
    Ee224,
    /// eego EE-225 (64 channels).
    Ee225,
    /// eego EE-511 (256 channels).
    Ee511,
}

impl AntNeuroVariant {
    /// Get the number of EEG channels for this variant.
    pub fn num_eeg_channels(&self) -> usize {
        match self {
            AntNeuroVariant::Ee410 => 64,
            AntNeuroVariant::Ee411 => 32,
            AntNeuroVariant::Ee430 => 128,
            AntNeuroVariant::Ee211 | AntNeuroVariant::Ee221 => 8,
            AntNeuroVariant::Ee212 | AntNeuroVariant::Ee222 => 16,
            AntNeuroVariant::Ee213 | AntNeuroVariant::Ee223 => 24,
            AntNeuroVariant::Ee214 | AntNeuroVariant::Ee224 => 32,
            AntNeuroVariant::Ee215 | AntNeuroVariant::Ee225 => 64,
            AntNeuroVariant::Ee511 => 256,
        }
    }

    /// Get the sampling rate for this variant.
    pub fn sampling_rate(&self) -> u32 {
        match self {
            AntNeuroVariant::Ee410 | AntNeuroVariant::Ee411 | AntNeuroVariant::Ee430 => 2048,
            AntNeuroVariant::Ee211
            | AntNeuroVariant::Ee212
            | AntNeuroVariant::Ee213
            | AntNeuroVariant::Ee214
            | AntNeuroVariant::Ee215 => 2048,
            AntNeuroVariant::Ee221
            | AntNeuroVariant::Ee222
            | AntNeuroVariant::Ee223
            | AntNeuroVariant::Ee224
            | AntNeuroVariant::Ee225 => 2048,
            AntNeuroVariant::Ee511 => 2048,
        }
    }

    /// Get the device name.
    pub fn name(&self) -> &'static str {
        match self {
            AntNeuroVariant::Ee410 => "ANT Neuro EE-410",
            AntNeuroVariant::Ee411 => "ANT Neuro EE-411",
            AntNeuroVariant::Ee430 => "ANT Neuro EE-430",
            AntNeuroVariant::Ee211 => "ANT Neuro EE-211",
            AntNeuroVariant::Ee212 => "ANT Neuro EE-212",
            AntNeuroVariant::Ee213 => "ANT Neuro EE-213",
            AntNeuroVariant::Ee214 => "ANT Neuro EE-214",
            AntNeuroVariant::Ee215 => "ANT Neuro EE-215",
            AntNeuroVariant::Ee221 => "ANT Neuro EE-221",
            AntNeuroVariant::Ee222 => "ANT Neuro EE-222",
            AntNeuroVariant::Ee223 => "ANT Neuro EE-223",
            AntNeuroVariant::Ee224 => "ANT Neuro EE-224",
            AntNeuroVariant::Ee225 => "ANT Neuro EE-225",
            AntNeuroVariant::Ee511 => "ANT Neuro EE-511",
        }
    }
}

/// ANT Neuro board implementation.
///
/// This board uses the eego SDK for communication. The actual hardware
/// connection would be established through USB.
pub struct AntNeuroBoard {
    state: BoardState,
    variant: AntNeuroVariant,
    descriptions: [BoardDescription; 3],
    buffers: [Option<RingBuffer<f64>>; 3],
    marker_queue: VecDeque<f64>,
    running: AtomicBool,
    sample_count: u64,
}

impl AntNeuroBoard {
    /// Create a new ANT Neuro board with the specified variant.
    pub fn new(variant: AntNeuroVariant) -> Self {
        let desc = Self::create_description(variant);

        Self {
            state: BoardState::NotReady,
            variant,
            descriptions: [desc.clone(), BoardDescription::default(), BoardDescription::default()],
            buffers: [None, None, None],
            marker_queue: VecDeque::new(),
            running: AtomicBool::new(false),
            sample_count: 0,
        }
    }

    fn create_description(variant: AntNeuroVariant) -> BoardDescription {
        let num_eeg = variant.num_eeg_channels();
        // Channels: package_num, EEG channels, timestamp, marker
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
            name: String::from(variant.name()),
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
    pub fn variant(&self) -> AntNeuroVariant {
        self.variant
    }
}

impl Board for AntNeuroBoard {
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

        // In a real implementation, we would initialize the eego SDK here
        // and connect to the amplifier

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
        self.sample_count = 0;

        Ok(())
    }

    fn start_stream(&mut self, buffer_size: usize) -> Result<()> {
        if self.state != BoardState::Ready {
            return Err(Error::new(ErrorCode::BoardNotReady));
        }

        let desc = &self.descriptions[0];
        self.buffers[0] = Some(RingBuffer::new(buffer_size, desc.num_channels));

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
        // ANT Neuro configuration would be done through the eego SDK
        Ok(String::new())
    }
}

// Type aliases for specific variants
/// ANT Neuro EE-410 (64 channels).
pub type AntNeuroEe410Board = AntNeuroBoard;

/// ANT Neuro EE-411 (32 channels).
pub type AntNeuroEe411Board = AntNeuroBoard;

/// ANT Neuro EE-430 (128 channels).
pub type AntNeuroEe430Board = AntNeuroBoard;

/// ANT Neuro EE-211 (8 channels).
pub type AntNeuroEe211Board = AntNeuroBoard;

/// ANT Neuro EE-212 (16 channels).
pub type AntNeuroEe212Board = AntNeuroBoard;

/// ANT Neuro EE-213 (24 channels).
pub type AntNeuroEe213Board = AntNeuroBoard;

/// ANT Neuro EE-214 (32 channels).
pub type AntNeuroEe214Board = AntNeuroBoard;

/// ANT Neuro EE-215 (64 channels).
pub type AntNeuroEe215Board = AntNeuroBoard;

/// ANT Neuro EE-221 (8 channels).
pub type AntNeuroEe221Board = AntNeuroBoard;

/// ANT Neuro EE-222 (16 channels).
pub type AntNeuroEe222Board = AntNeuroBoard;

/// ANT Neuro EE-223 (24 channels).
pub type AntNeuroEe223Board = AntNeuroBoard;

/// ANT Neuro EE-224 (32 channels).
pub type AntNeuroEe224Board = AntNeuroBoard;

/// ANT Neuro EE-225 (64 channels).
pub type AntNeuroEe225Board = AntNeuroBoard;

/// ANT Neuro EE-511 (256 channels).
pub type AntNeuroEe511Board = AntNeuroBoard;

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_variant_channels() {
        assert_eq!(AntNeuroVariant::Ee410.num_eeg_channels(), 64);
        assert_eq!(AntNeuroVariant::Ee411.num_eeg_channels(), 32);
        assert_eq!(AntNeuroVariant::Ee430.num_eeg_channels(), 128);
        assert_eq!(AntNeuroVariant::Ee211.num_eeg_channels(), 8);
        assert_eq!(AntNeuroVariant::Ee511.num_eeg_channels(), 256);
    }

    #[test]
    fn test_ant_neuro_board_creation() {
        let board = AntNeuroBoard::new(AntNeuroVariant::Ee410);
        assert_eq!(board.state(), BoardState::NotReady);

        let desc = board.description(Preset::Default);
        assert_eq!(desc.sampling_rate, 2048);
        assert_eq!(desc.eeg_channels.len(), 64);
    }

    #[test]
    fn test_ant_neuro_lifecycle() {
        let mut board = AntNeuroBoard::new(AntNeuroVariant::Ee211);

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
