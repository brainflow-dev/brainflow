//! Galea board implementations.
//!
//! Galea is a high-channel-count EEG/EMG/ECG board that streams data over UDP.

use alloc::boxed::Box;
use alloc::collections::VecDeque;
use alloc::string::String;
use alloc::vec::Vec;
use core::sync::atomic::{AtomicBool, AtomicU64, Ordering};

use brainflow_io::{IoError, RingBuffer, UdpSocket};
use brainflow_sys::error::{Error, ErrorCode, Result};

use crate::board::{Board, BoardDescription, BoardState, reshape_to_row_major};
use crate::preset::Preset;

/// Galea default UDP port for data streaming.
const GALEA_UDP_PORT: u16 = 2390;

/// Galea packet header.
const GALEA_HEADER: u8 = 0xA0;

/// Galea packet size.
const GALEA_PACKET_SIZE: usize = 72;

/// Galea board (high-channel-count EEG/EMG/ECG).
pub struct GaleaBoard {
    state: BoardState,
    descriptions: [BoardDescription; 3],
    buffers: [Option<RingBuffer<f64>>; 3],
    marker_queue: VecDeque<f64>,
    udp_socket: Option<Box<dyn UdpSocket>>,
    ip_address: String,
    ip_port: u16,
    running: AtomicBool,
    sample_count: AtomicU64,
    read_buffer: Vec<u8>,
}

impl GaleaBoard {
    /// Galea sampling rate.
    pub const SAMPLING_RATE: u32 = 250;

    /// Number of EEG channels.
    pub const NUM_EEG_CHANNELS: usize = 8;

    /// Number of EMG channels.
    pub const NUM_EMG_CHANNELS: usize = 4;

    /// Number of other channels.
    pub const NUM_OTHER_CHANNELS: usize = 4;

    /// EEG scale factor.
    pub const EEG_SCALE: f64 = 4.5 / 8388607.0 / 24.0 * 1_000_000.0;

    /// Create a new Galea board.
    pub fn new(ip_address: &str, ip_port: Option<u16>) -> Self {
        Self {
            state: BoardState::NotReady,
            descriptions: [
                Self::create_default_description(),
                Self::create_auxiliary_description(),
                Self::create_ancillary_description(),
            ],
            buffers: [None, None, None],
            marker_queue: VecDeque::new(),
            udp_socket: None,
            ip_address: String::from(ip_address),
            ip_port: ip_port.unwrap_or(GALEA_UDP_PORT),
            running: AtomicBool::new(false),
            sample_count: AtomicU64::new(0),
            read_buffer: Vec::with_capacity(4096),
        }
    }

    fn create_default_description() -> BoardDescription {
        // Default preset: EEG, EMG, and auxiliary channels
        let num_channels = 21; // package + 8 EEG + 4 EMG + 4 other + 3 accel + timestamp

        BoardDescription {
            sampling_rate: Self::SAMPLING_RATE,
            num_channels,
            eeg_channels: (1..=Self::NUM_EEG_CHANNELS).collect(),
            emg_channels: (9..=12).collect(),
            ecg_channels: Vec::new(),
            eog_channels: Vec::new(),
            ppg_channels: alloc::vec![13, 14],
            eda_channels: alloc::vec![15, 16],
            accel_channels: alloc::vec![17, 18, 19],
            gyro_channels: Vec::new(),
            temperature_channels: Vec::new(),
            battery_channel: None,
            timestamp_channel: Some(20),
            marker_channel: None,
            package_num_channel: Some(0),
            name: String::from("Galea"),
        }
    }

    fn create_auxiliary_description() -> BoardDescription {
        // Auxiliary preset: Accelerometer and gyro
        BoardDescription {
            sampling_rate: Self::SAMPLING_RATE,
            num_channels: 7, // package + 3 accel + 3 gyro
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
            name: String::from("Galea Aux"),
        }
    }

    fn create_ancillary_description() -> BoardDescription {
        // Ancillary preset: PPG, EDA
        BoardDescription {
            sampling_rate: Self::SAMPLING_RATE,
            num_channels: 5, // package + 2 PPG + 2 EDA
            eeg_channels: Vec::new(),
            emg_channels: Vec::new(),
            ecg_channels: Vec::new(),
            eog_channels: Vec::new(),
            ppg_channels: alloc::vec![1, 2],
            eda_channels: alloc::vec![3, 4],
            accel_channels: Vec::new(),
            gyro_channels: Vec::new(),
            temperature_channels: Vec::new(),
            battery_channel: None,
            timestamp_channel: None,
            marker_channel: None,
            package_num_channel: Some(0),
            name: String::from("Galea Ancillary"),
        }
    }

    fn preset_index(preset: Preset) -> usize {
        preset.to_raw() as usize
    }

    /// Parse a Galea packet from raw bytes.
    fn parse_packet(&self, data: &[u8]) -> Option<Vec<f64>> {
        if data.len() < GALEA_PACKET_SIZE {
            return None;
        }

        if data[0] != GALEA_HEADER {
            return None;
        }

        let sample_number = data[1];
        let num_channels = self.descriptions[0].num_channels;
        let mut sample = Vec::with_capacity(num_channels);

        // Package number
        sample.push(sample_number as f64);

        // Parse EEG channels (24-bit signed, MSB first)
        for i in 0..Self::NUM_EEG_CHANNELS {
            let offset = 2 + i * 3;
            let raw = parse_int24(&data[offset..offset + 3]);
            sample.push(raw as f64 * Self::EEG_SCALE);
        }

        // Parse EMG channels (24-bit signed)
        for i in 0..Self::NUM_EMG_CHANNELS {
            let offset = 26 + i * 3;
            let raw = parse_int24(&data[offset..offset + 3]);
            sample.push(raw as f64 * Self::EEG_SCALE);
        }

        // Parse other channels (PPG, EDA) - 16-bit
        for i in 0..Self::NUM_OTHER_CHANNELS {
            let offset = 38 + i * 2;
            let raw = parse_int16(&data[offset..offset + 2]);
            sample.push(raw as f64);
        }

        // Parse accelerometer (16-bit)
        for i in 0..3 {
            let offset = 46 + i * 2;
            let raw = parse_int16(&data[offset..offset + 2]);
            sample.push(raw as f64 * 0.002 / 16.0); // Accelerometer scale
        }

        // Timestamp
        let pkg = self.sample_count.load(Ordering::Relaxed);
        sample.push(pkg as f64 / Self::SAMPLING_RATE as f64);

        Some(sample)
    }

    /// Read UDP data.
    fn read_udp_data(&mut self) -> Result<usize> {
        if let Some(ref mut socket) = self.udp_socket {
            let mut temp_buf = [0u8; 2048];
            match socket.recv(&mut temp_buf) {
                Ok(n) if n > 0 => {
                    self.read_buffer.extend_from_slice(&temp_buf[..n]);
                    Ok(n)
                }
                Ok(_) => Ok(0),
                Err(IoError::Timeout) => Ok(0),
                Err(_) => Err(Error::new(ErrorCode::FailedToReceiveData)),
            }
        } else {
            Err(Error::new(ErrorCode::BoardNotReady))
        }
    }

    /// Process received packets.
    fn process_packets(&mut self) -> usize {
        let mut packets_processed = 0;

        loop {
            // Find header
            let header_pos = self.read_buffer.iter().position(|&b| b == GALEA_HEADER);

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

            if self.read_buffer.len() < GALEA_PACKET_SIZE {
                break;
            }

            if let Some(sample) = self.parse_packet(&self.read_buffer[..GALEA_PACKET_SIZE]) {
                self.sample_count.fetch_add(1, Ordering::Relaxed);

                // Push to default buffer
                if let Some(ref buffer) = self.buffers[0] {
                    buffer.push(&sample);
                }

                // Extract auxiliary data for aux buffer
                if let Some(ref buffer) = self.buffers[1] {
                    // Aux: package + accel + gyro
                    let aux_sample = alloc::vec![
                        sample[0],                                    // package
                        sample.get(17).copied().unwrap_or(0.0),       // accel_x
                        sample.get(18).copied().unwrap_or(0.0),       // accel_y
                        sample.get(19).copied().unwrap_or(0.0),       // accel_z
                        0.0,                                          // gyro_x (placeholder)
                        0.0,                                          // gyro_y
                        0.0,                                          // gyro_z
                    ];
                    buffer.push(&aux_sample);
                }

                // Extract ancillary data for ancillary buffer
                if let Some(ref buffer) = self.buffers[2] {
                    // Ancillary: package + PPG + EDA
                    let anc_sample = alloc::vec![
                        sample[0],                                    // package
                        sample.get(13).copied().unwrap_or(0.0),       // ppg1
                        sample.get(14).copied().unwrap_or(0.0),       // ppg2
                        sample.get(15).copied().unwrap_or(0.0),       // eda1
                        sample.get(16).copied().unwrap_or(0.0),       // eda2
                    ];
                    buffer.push(&anc_sample);
                }

                packets_processed += 1;
            }

            self.read_buffer.drain(..GALEA_PACKET_SIZE);
        }

        packets_processed
    }

    /// Process incoming data.
    pub fn process(&mut self) -> Result<usize> {
        if !self.running.load(Ordering::Acquire) {
            return Ok(0);
        }

        self.read_udp_data()?;
        Ok(self.process_packets())
    }
}

impl Board for GaleaBoard {
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

        // In a real implementation, we would:
        // 1. Create UDP socket
        // 2. Bind to local port
        // 3. Connect to device IP

        self.state = BoardState::Ready;
        Ok(())
    }

    fn release_session(&mut self) -> Result<()> {
        if self.state == BoardState::Streaming {
            self.stop_stream()?;
        }

        if let Some(ref mut socket) = self.udp_socket {
            let _ = socket.close();
        }
        self.udp_socket = None;

        self.state = BoardState::NotReady;
        self.buffers = [None, None, None];
        self.marker_queue.clear();
        self.read_buffer.clear();
        self.sample_count.store(0, Ordering::Relaxed);

        Ok(())
    }

    fn start_stream(&mut self, buffer_size: usize) -> Result<()> {
        if self.state != BoardState::Ready {
            return Err(Error::new(ErrorCode::BoardNotReady));
        }

        // Create ring buffers
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
        // Galea config would be sent via separate protocol
        Ok(String::from("OK"))
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

/// Parse 16-bit signed integer (MSB first).
fn parse_int16(data: &[u8]) -> i16 {
    if data.len() < 2 {
        return 0;
    }

    ((data[0] as i16) << 8) | (data[1] as i16)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_galea_board_creation() {
        let board = GaleaBoard::new("192.168.1.100", None);
        assert_eq!(board.state(), BoardState::NotReady);

        let desc = board.description(Preset::Default);
        assert_eq!(desc.sampling_rate, 250);
        assert_eq!(desc.eeg_channels.len(), 8);
        assert_eq!(desc.emg_channels.len(), 4);
    }

    #[test]
    fn test_galea_auxiliary_preset() {
        let board = GaleaBoard::new("192.168.1.100", None);

        let desc = board.description(Preset::Auxiliary);
        assert_eq!(desc.accel_channels.len(), 3);
        assert_eq!(desc.gyro_channels.len(), 3);
    }

    #[test]
    fn test_galea_ancillary_preset() {
        let board = GaleaBoard::new("192.168.1.100", None);

        let desc = board.description(Preset::Ancillary);
        assert_eq!(desc.ppg_channels.len(), 2);
        assert_eq!(desc.eda_channels.len(), 2);
    }

    #[test]
    fn test_galea_lifecycle() {
        let mut board = GaleaBoard::new("192.168.1.100", None);

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
    fn test_parse_int24() {
        // Positive value
        let data = [0x00, 0x00, 0x01];
        assert_eq!(parse_int24(&data), 1);

        // Negative value
        let data = [0xFF, 0xFF, 0xFF];
        assert_eq!(parse_int24(&data), -1);
    }
}
