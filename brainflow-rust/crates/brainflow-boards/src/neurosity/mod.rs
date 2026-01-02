//! Neurosity board implementations.
//!
//! Supports Neurosity Crown and Notion headsets using OSC protocol.

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

/// Neurosity device variant.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum NeurosityVariant {
    /// Neurosity Notion 1 (8 EEG channels).
    Notion1,
    /// Neurosity Notion 2 (8 EEG channels).
    Notion2,
    /// Neurosity Crown (8 EEG channels, updated design).
    Crown,
}

impl NeurosityVariant {
    /// Get the number of EEG channels.
    pub fn num_eeg_channels(&self) -> usize {
        8 // All variants have 8 channels
    }

    /// Get the sampling rate.
    pub fn sampling_rate(&self) -> u32 {
        256 // All variants sample at 256 Hz
    }

    /// Get the device name.
    pub fn name(&self) -> &'static str {
        match self {
            NeurosityVariant::Notion1 => "Neurosity Notion 1",
            NeurosityVariant::Notion2 => "Neurosity Notion 2",
            NeurosityVariant::Crown => "Neurosity Crown",
        }
    }

    /// Get the default OSC port for this variant.
    pub fn default_osc_port(&self) -> u16 {
        match self {
            NeurosityVariant::Notion1 => 9000,
            NeurosityVariant::Notion2 => 9000,
            NeurosityVariant::Crown => 9000,
        }
    }
}

/// Neurosity board implementation using OSC protocol.
///
/// OSC (Open Sound Control) messages are received over UDP.
/// The device sends EEG data as OSC bundles.
pub struct NeurosityBoard {
    state: BoardState,
    variant: NeurosityVariant,
    descriptions: [BoardDescription; 3],
    buffers: [Option<RingBuffer<f64>>; 3],
    marker_queue: VecDeque<f64>,
    running: AtomicBool,
    sample_count: u64,
    ip_address: String,
    port: u16,
}

impl NeurosityBoard {
    /// Create a new Neurosity board.
    pub fn new(variant: NeurosityVariant, ip_address: Option<&str>, port: Option<u16>) -> Self {
        let desc = Self::create_description(variant);

        Self {
            state: BoardState::NotReady,
            variant,
            descriptions: [desc.clone(), BoardDescription::default(), BoardDescription::default()],
            buffers: [None, None, None],
            marker_queue: VecDeque::new(),
            running: AtomicBool::new(false),
            sample_count: 0,
            ip_address: String::from(ip_address.unwrap_or("127.0.0.1")),
            port: port.unwrap_or(variant.default_osc_port()),
        }
    }

    fn create_description(variant: NeurosityVariant) -> BoardDescription {
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
    pub fn variant(&self) -> NeurosityVariant {
        self.variant
    }

    /// Create a Notion 1 board.
    pub fn notion1() -> Self {
        Self::new(NeurosityVariant::Notion1, None, None)
    }

    /// Create a Notion 2 board.
    pub fn notion2() -> Self {
        Self::new(NeurosityVariant::Notion2, None, None)
    }

    /// Create a Crown board.
    pub fn crown() -> Self {
        Self::new(NeurosityVariant::Crown, None, None)
    }

    /// Parse an OSC message containing EEG data.
    ///
    /// OSC message format: /muse/eeg [f32, f32, ...]
    #[allow(dead_code)]
    fn parse_osc_message(&self, data: &[u8]) -> Option<Vec<f64>> {
        // OSC message structure:
        // - Address pattern (null-terminated, padded to 4-byte boundary)
        // - Type tag string (null-terminated, padded to 4-byte boundary)
        // - Arguments

        if data.len() < 16 {
            return None;
        }

        // Find the address pattern end
        let addr_end = data.iter().position(|&b| b == 0)?;
        let addr = core::str::from_utf8(&data[..addr_end]).ok()?;

        // Check if this is an EEG message
        if !addr.contains("eeg") && !addr.contains("raw") {
            return None;
        }

        // Find type tag string (starts with ',')
        let type_start = ((addr_end + 4) / 4) * 4; // Align to 4 bytes
        if type_start >= data.len() || data[type_start] != b',' {
            return None;
        }

        let type_end = data[type_start..].iter().position(|&b| b == 0)?;
        let type_tag = core::str::from_utf8(&data[type_start..type_start + type_end]).ok()?;

        // Parse float arguments
        let args_start = ((type_start + type_end + 4) / 4) * 4;
        let num_floats = type_tag.chars().filter(|&c| c == 'f').count();

        let mut values = Vec::with_capacity(num_floats);
        let mut offset = args_start;

        for _ in 0..num_floats {
            if offset + 4 > data.len() {
                break;
            }
            let bytes: [u8; 4] = data[offset..offset + 4].try_into().ok()?;
            // OSC uses big-endian
            let value = f32::from_be_bytes(bytes);
            values.push(value as f64);
            offset += 4;
        }

        Some(values)
    }
}

impl Board for NeurosityBoard {
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

        // In a real implementation, we would set up UDP socket for OSC

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
        Ok(String::new())
    }
}

// Type aliases for specific variants
/// Neurosity Notion 1 board.
pub type Notion1Board = NeurosityBoard;

/// Neurosity Notion 2 board.
pub type Notion2Board = NeurosityBoard;

/// Neurosity Crown board.
pub type CrownBoard = NeurosityBoard;

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_variant_channels() {
        assert_eq!(NeurosityVariant::Notion1.num_eeg_channels(), 8);
        assert_eq!(NeurosityVariant::Notion2.num_eeg_channels(), 8);
        assert_eq!(NeurosityVariant::Crown.num_eeg_channels(), 8);
    }

    #[test]
    fn test_neurosity_board_creation() {
        let board = NeurosityBoard::crown();
        assert_eq!(board.state(), BoardState::NotReady);

        let desc = board.description(Preset::Default);
        assert_eq!(desc.sampling_rate, 256);
        assert_eq!(desc.eeg_channels.len(), 8);
    }

    #[test]
    fn test_neurosity_lifecycle() {
        let mut board = NeurosityBoard::notion1();

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
