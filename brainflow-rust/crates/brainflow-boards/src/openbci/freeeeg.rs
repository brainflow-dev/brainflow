//! FreeEEG32 board implementation.
//!
//! FreeEEG32 is a 32-channel open-source EEG board using 4× ADS1299 chips.
//! It communicates over serial at 921600 baud.

use alloc::boxed::Box;
use alloc::collections::VecDeque;
use alloc::string::String;
use alloc::vec::Vec;
use core::sync::atomic::{AtomicBool, Ordering};

use brainflow_io::{IoError, RingBuffer, SerialConfig, SerialPort};
use brainflow_sys::error::{Error, ErrorCode, Result};

use crate::board::{Board, BoardDescription, BoardState, reshape_to_row_major};
use crate::preset::Preset;

/// FreeEEG32 packet header byte.
const HEADER_BYTE: u8 = 0xA0;

/// FreeEEG32 packet size: 1 header + 1 sample + 96 data (32 ch × 3 bytes) + 1 footer = 99 bytes
const PACKET_SIZE: usize = 99;

/// Number of EEG channels.
const NUM_EEG_CHANNELS: usize = 32;

/// EEG scale factor: 4.5V / (2^23 - 1) / gain * 1e6 μV
const EEG_SCALE_24: f64 = 4.5 / 8388607.0 / 24.0 * 1_000_000.0;

/// FreeEEG32 board (32-channel EEG).
pub struct FreeEeg32Board {
    state: BoardState,
    descriptions: [BoardDescription; 3],
    buffers: [Option<RingBuffer<f64>>; 3],
    marker_queue: VecDeque<f64>,
    serial_port: Option<Box<dyn SerialPort>>,
    port_name: String,
    gain: f64,
    running: AtomicBool,
    sample_count: u64,
    read_buffer: Vec<u8>,
}

impl FreeEeg32Board {
    /// FreeEEG32 sampling rate.
    pub const SAMPLING_RATE: u32 = 256;

    /// Serial baud rate.
    pub const BAUD_RATE: u32 = 921600;

    /// Number of EEG channels.
    pub const NUM_EEG_CHANNELS: usize = NUM_EEG_CHANNELS;

    /// Create a new FreeEEG32 board.
    pub fn new(port_name: &str) -> Self {
        Self {
            state: BoardState::NotReady,
            descriptions: [
                Self::create_default_description(),
                BoardDescription::default(),
                BoardDescription::default(),
            ],
            buffers: [None, None, None],
            marker_queue: VecDeque::new(),
            serial_port: None,
            port_name: String::from(port_name),
            gain: 24.0,
            running: AtomicBool::new(false),
            sample_count: 0,
            read_buffer: Vec::with_capacity(4096),
        }
    }

    fn create_default_description() -> BoardDescription {
        // Channels: package_num, 32 EEG, timestamp, marker = 35
        let num_channels = 35;

        BoardDescription {
            sampling_rate: Self::SAMPLING_RATE,
            num_channels,
            eeg_channels: (1..=NUM_EEG_CHANNELS).collect(),
            emg_channels: Vec::new(),
            ecg_channels: Vec::new(),
            eog_channels: Vec::new(),
            ppg_channels: Vec::new(),
            eda_channels: Vec::new(),
            accel_channels: Vec::new(),
            gyro_channels: Vec::new(),
            temperature_channels: Vec::new(),
            battery_channel: None,
            timestamp_channel: Some(33),
            marker_channel: Some(34),
            package_num_channel: Some(0),
            name: String::from("FreeEEG32"),
        }
    }

    fn preset_index(preset: Preset) -> usize {
        preset.to_raw() as usize
    }

    /// Parse a FreeEEG32 packet from raw bytes.
    fn parse_packet(&self, data: &[u8]) -> Option<Vec<f64>> {
        if data.len() < PACKET_SIZE {
            return None;
        }

        if data[0] != HEADER_BYTE {
            return None;
        }

        let sample_number = data[1];
        let scale = 4.5 / 8388607.0 / self.gain * 1_000_000.0;

        // Create sample vector
        let mut sample = Vec::with_capacity(35);

        // Package number
        sample.push(sample_number as f64);

        // Parse 32 EEG channels (24-bit signed integers, MSB first)
        for i in 0..NUM_EEG_CHANNELS {
            let offset = 2 + i * 3;
            let raw = parse_int24(&data[offset..offset + 3]);
            sample.push(raw as f64 * scale);
        }

        // Timestamp
        self.sample_count;
        let timestamp = self.sample_count as f64 / Self::SAMPLING_RATE as f64;
        sample.push(timestamp);

        // Marker
        let marker = self.marker_queue.front().copied().unwrap_or(0.0);
        sample.push(marker);

        Some(sample)
    }

    /// Read and process data from serial port.
    pub fn process(&mut self) -> Result<usize> {
        if !self.running.load(Ordering::Acquire) {
            return Ok(0);
        }

        // Read available data
        if let Some(ref mut port) = self.serial_port {
            let mut temp_buf = [0u8; 1024];
            match port.read(&mut temp_buf) {
                Ok(n) if n > 0 => {
                    self.read_buffer.extend_from_slice(&temp_buf[..n]);
                }
                Ok(_) => {}
                Err(IoError::Timeout) => {}
                Err(_) => return Err(Error::new(ErrorCode::FailedToReceiveData)),
            }
        }

        // Process packets
        let mut packets_processed = 0;

        loop {
            // Find header
            let header_pos = self.read_buffer.iter().position(|&b| b == HEADER_BYTE);

            match header_pos {
                Some(pos) if pos > 0 => {
                    self.read_buffer.drain(..pos);
                }
                None => {
                    self.read_buffer.clear();
                    break;
                }
                _ => {}
            }

            if self.read_buffer.len() < PACKET_SIZE {
                break;
            }

            if let Some(mut sample) = self.parse_packet(&self.read_buffer[..PACKET_SIZE]) {
                // Update marker
                if !self.marker_queue.is_empty() {
                    self.marker_queue.pop_front();
                }

                // Update sample count and timestamp
                self.sample_count += 1;
                if sample.len() > 33 {
                    sample[33] = self.sample_count as f64 / Self::SAMPLING_RATE as f64;
                }

                // Push to buffer
                if let Some(ref buffer) = self.buffers[0] {
                    buffer.push(&sample);
                }

                packets_processed += 1;
            }

            self.read_buffer.drain(..PACKET_SIZE);
        }

        Ok(packets_processed)
    }
}

impl Board for FreeEeg32Board {
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

        if let Some(ref mut port) = self.serial_port {
            let _ = port.close();
        }
        self.serial_port = None;

        self.state = BoardState::NotReady;
        self.buffers = [None, None, None];
        self.marker_queue.clear();
        self.read_buffer.clear();
        self.sample_count = 0;

        Ok(())
    }

    fn start_stream(&mut self, buffer_size: usize) -> Result<()> {
        if self.state != BoardState::Ready {
            return Err(Error::new(ErrorCode::BoardNotReady));
        }

        // Create ring buffer
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

    fn config_board(&mut self, config: &str) -> Result<String> {
        // FreeEEG32 accepts raw command strings
        if let Some(ref mut port) = self.serial_port {
            port.write(config.as_bytes())
                .map_err(|_| Error::new(ErrorCode::FailedToSendData))?;
            port.flush()
                .map_err(|_| Error::new(ErrorCode::FailedToSendData))?;
        }
        Ok(String::from("Command sent"))
    }
}

/// Parse 24-bit signed integer (MSB first).
fn parse_int24(data: &[u8]) -> i32 {
    if data.len() < 3 {
        return 0;
    }

    let unsigned = ((data[0] as u32) << 16) | ((data[1] as u32) << 8) | (data[2] as u32);

    if unsigned & 0x800000 != 0 {
        (unsigned | 0xFF000000) as i32
    } else {
        unsigned as i32
    }
}

/// FreeEEG128 board (128-channel EEG using 16× ADS1299).
pub struct FreeEeg128Board {
    state: BoardState,
    descriptions: [BoardDescription; 3],
    buffers: [Option<RingBuffer<f64>>; 3],
    marker_queue: VecDeque<f64>,
    serial_port: Option<Box<dyn SerialPort>>,
    port_name: String,
    gain: f64,
    running: AtomicBool,
    sample_count: u64,
    read_buffer: Vec<u8>,
}

impl FreeEeg128Board {
    /// FreeEEG128 sampling rate.
    pub const SAMPLING_RATE: u32 = 256;

    /// Serial baud rate.
    pub const BAUD_RATE: u32 = 921600;

    /// Number of EEG channels.
    pub const NUM_EEG_CHANNELS: usize = 128;

    /// Packet size: 1 header + 1 sample + 384 data (128 ch × 3 bytes) + 1 footer = 387 bytes
    const PACKET_SIZE: usize = 387;

    /// Create a new FreeEEG128 board.
    pub fn new(port_name: &str) -> Self {
        Self {
            state: BoardState::NotReady,
            descriptions: [
                Self::create_default_description(),
                BoardDescription::default(),
                BoardDescription::default(),
            ],
            buffers: [None, None, None],
            marker_queue: VecDeque::new(),
            serial_port: None,
            port_name: String::from(port_name),
            gain: 24.0,
            running: AtomicBool::new(false),
            sample_count: 0,
            read_buffer: Vec::with_capacity(8192),
        }
    }

    fn create_default_description() -> BoardDescription {
        // Channels: package_num, 128 EEG, timestamp, marker = 131
        let num_channels = 131;

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
            battery_channel: None,
            timestamp_channel: Some(129),
            marker_channel: Some(130),
            package_num_channel: Some(0),
            name: String::from("FreeEEG128"),
        }
    }

    fn preset_index(preset: Preset) -> usize {
        preset.to_raw() as usize
    }

    /// Parse a FreeEEG128 packet from raw bytes.
    fn parse_packet(&self, data: &[u8]) -> Option<Vec<f64>> {
        if data.len() < Self::PACKET_SIZE {
            return None;
        }

        if data[0] != HEADER_BYTE {
            return None;
        }

        let sample_number = data[1];
        let scale = 4.5 / 8388607.0 / self.gain * 1_000_000.0;

        let mut sample = Vec::with_capacity(131);

        // Package number
        sample.push(sample_number as f64);

        // Parse 128 EEG channels (24-bit signed integers, MSB first)
        for i in 0..Self::NUM_EEG_CHANNELS {
            let offset = 2 + i * 3;
            let raw = parse_int24(&data[offset..offset + 3]);
            sample.push(raw as f64 * scale);
        }

        // Timestamp
        let timestamp = self.sample_count as f64 / Self::SAMPLING_RATE as f64;
        sample.push(timestamp);

        // Marker
        let marker = self.marker_queue.front().copied().unwrap_or(0.0);
        sample.push(marker);

        Some(sample)
    }

    /// Read and process data from serial port.
    pub fn process(&mut self) -> Result<usize> {
        if !self.running.load(Ordering::Acquire) {
            return Ok(0);
        }

        if let Some(ref mut port) = self.serial_port {
            let mut temp_buf = [0u8; 2048];
            match port.read(&mut temp_buf) {
                Ok(n) if n > 0 => {
                    self.read_buffer.extend_from_slice(&temp_buf[..n]);
                }
                Ok(_) => {}
                Err(IoError::Timeout) => {}
                Err(_) => return Err(Error::new(ErrorCode::FailedToReceiveData)),
            }
        }

        let mut packets_processed = 0;

        loop {
            let header_pos = self.read_buffer.iter().position(|&b| b == HEADER_BYTE);

            match header_pos {
                Some(pos) if pos > 0 => {
                    self.read_buffer.drain(..pos);
                }
                None => {
                    self.read_buffer.clear();
                    break;
                }
                _ => {}
            }

            if self.read_buffer.len() < Self::PACKET_SIZE {
                break;
            }

            if let Some(mut sample) = self.parse_packet(&self.read_buffer[..Self::PACKET_SIZE]) {
                if !self.marker_queue.is_empty() {
                    self.marker_queue.pop_front();
                }

                self.sample_count += 1;
                if sample.len() > 129 {
                    sample[129] = self.sample_count as f64 / Self::SAMPLING_RATE as f64;
                }

                if let Some(ref buffer) = self.buffers[0] {
                    buffer.push(&sample);
                }

                packets_processed += 1;
            }

            self.read_buffer.drain(..Self::PACKET_SIZE);
        }

        Ok(packets_processed)
    }
}

impl Board for FreeEeg128Board {
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

        if let Some(ref mut port) = self.serial_port {
            let _ = port.close();
        }
        self.serial_port = None;

        self.state = BoardState::NotReady;
        self.buffers = [None, None, None];
        self.marker_queue.clear();
        self.read_buffer.clear();
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

    fn config_board(&mut self, config: &str) -> Result<String> {
        if let Some(ref mut port) = self.serial_port {
            port.write(config.as_bytes())
                .map_err(|_| Error::new(ErrorCode::FailedToSendData))?;
            port.flush()
                .map_err(|_| Error::new(ErrorCode::FailedToSendData))?;
        }
        Ok(String::from("Command sent"))
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_freeeeg32_creation() {
        let board = FreeEeg32Board::new("/dev/ttyUSB0");
        assert_eq!(board.state(), BoardState::NotReady);

        let desc = board.description(Preset::Default);
        assert_eq!(desc.sampling_rate, 256);
        assert_eq!(desc.eeg_channels.len(), 32);
    }

    #[test]
    fn test_freeeeg32_lifecycle() {
        let mut board = FreeEeg32Board::new("/dev/ttyUSB0");

        board.prepare_session().unwrap();
        assert_eq!(board.state(), BoardState::Ready);

        board.start_stream(1000).unwrap();
        assert_eq!(board.state(), BoardState::Streaming);

        board.stop_stream().unwrap();
        assert_eq!(board.state(), BoardState::Ready);

        board.release_session().unwrap();
        assert_eq!(board.state(), BoardState::NotReady);
    }

    #[test]
    fn test_freeeeg128_creation() {
        let board = FreeEeg128Board::new("/dev/ttyUSB0");
        assert_eq!(board.state(), BoardState::NotReady);

        let desc = board.description(Preset::Default);
        assert_eq!(desc.sampling_rate, 256);
        assert_eq!(desc.eeg_channels.len(), 128);
    }

    #[test]
    fn test_freeeeg128_lifecycle() {
        let mut board = FreeEeg128Board::new("/dev/ttyUSB0");

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
