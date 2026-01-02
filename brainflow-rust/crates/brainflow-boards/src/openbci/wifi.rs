//! OpenBCI WiFi board implementations.
//!
//! These boards use the OpenBCI WiFi Shield which communicates via HTTP
//! for configuration and streams data over TCP.

use alloc::boxed::Box;
use alloc::collections::VecDeque;
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use core::sync::atomic::{AtomicBool, Ordering};

use brainflow_io::{HttpClient, IoError, RingBuffer, SocketConfig, TcpClient};
use brainflow_sys::error::{Error, ErrorCode, Result};

use crate::board::{Board, BoardDescription, BoardState, reshape_to_row_major};
use crate::preset::Preset;

use super::packet::{OpenBciPacket, CYTON_PACKET_SIZE, HEADER_BYTE};

/// OpenBCI WiFi Shield default port.
const WIFI_DEFAULT_PORT: u16 = 80;

/// TCP streaming port.
const TCP_STREAM_PORT: u16 = 2000;

/// OpenBCI Cyton WiFi board.
pub struct CytonWifiBoard {
    state: BoardState,
    descriptions: [BoardDescription; 3],
    buffers: [Option<RingBuffer<f64>>; 3],
    marker_queue: VecDeque<f64>,
    http_client: Option<Box<dyn HttpClient>>,
    tcp_client: Option<Box<dyn TcpClient>>,
    ip_address: String,
    ip_port: u16,
    gain: f64,
    running: AtomicBool,
    sample_count: u64,
    read_buffer: Vec<u8>,
}

impl CytonWifiBoard {
    /// Cyton sampling rate.
    pub const SAMPLING_RATE: u32 = 250;

    /// Number of EEG channels.
    pub const NUM_EEG_CHANNELS: usize = 8;

    /// Default gain.
    pub const DEFAULT_GAIN: f64 = 24.0;

    /// Create a new Cyton WiFi board.
    pub fn new(ip_address: &str, ip_port: Option<u16>) -> Self {
        Self {
            state: BoardState::NotReady,
            descriptions: [
                Self::create_default_description(),
                Self::create_auxiliary_description(),
                BoardDescription::default(),
            ],
            buffers: [None, None, None],
            marker_queue: VecDeque::new(),
            http_client: None,
            tcp_client: None,
            ip_address: String::from(ip_address),
            ip_port: ip_port.unwrap_or(WIFI_DEFAULT_PORT),
            gain: Self::DEFAULT_GAIN,
            running: AtomicBool::new(false),
            sample_count: 0,
            read_buffer: Vec::with_capacity(4096),
        }
    }

    fn create_default_description() -> BoardDescription {
        let num_channels = 14; // package + 8 EEG + 3 accel + timestamp + marker

        BoardDescription {
            sampling_rate: Self::SAMPLING_RATE,
            num_channels,
            eeg_channels: (1..=Self::NUM_EEG_CHANNELS).collect(),
            emg_channels: Vec::new(),
            ecg_channels: Vec::new(),
            eog_channels: Vec::new(),
            ppg_channels: Vec::new(),
            eda_channels: Vec::new(),
            accel_channels: alloc::vec![9, 10, 11],
            gyro_channels: Vec::new(),
            temperature_channels: Vec::new(),
            battery_channel: Some(13),
            timestamp_channel: Some(11),
            marker_channel: Some(12),
            package_num_channel: Some(0),
            name: String::from("OpenBCI Cyton WiFi"),
        }
    }

    fn create_auxiliary_description() -> BoardDescription {
        BoardDescription {
            sampling_rate: Self::SAMPLING_RATE,
            num_channels: 4,
            eeg_channels: Vec::new(),
            emg_channels: Vec::new(),
            ecg_channels: Vec::new(),
            eog_channels: Vec::new(),
            ppg_channels: Vec::new(),
            eda_channels: Vec::new(),
            accel_channels: alloc::vec![1, 2, 3],
            gyro_channels: Vec::new(),
            temperature_channels: Vec::new(),
            battery_channel: None,
            timestamp_channel: None,
            marker_channel: None,
            package_num_channel: Some(0),
            name: String::from("OpenBCI Cyton WiFi Aux"),
        }
    }

    fn preset_index(preset: Preset) -> usize {
        preset.to_raw() as usize
    }

    /// Send HTTP command to the WiFi shield.
    #[cfg(feature = "std")]
    fn send_http_command(&self, endpoint: &str) -> Result<String> {
        if let Some(ref client) = self.http_client {
            use std::format;
            let url = format!("http://{}:{}{}", self.ip_address, self.ip_port, endpoint);
            let response = client
                .get(&url)
                .map_err(|_| Error::new(ErrorCode::FailedToSendData))?;

            if response.is_success() {
                Ok(response.body_str().unwrap_or("").to_string())
            } else {
                Err(Error::new(ErrorCode::GeneralError))
            }
        } else {
            Err(Error::new(ErrorCode::BoardNotReady))
        }
    }

    #[cfg(not(feature = "std"))]
    fn send_http_command(&self, _endpoint: &str) -> Result<String> {
        Err(Error::new(ErrorCode::GeneralError))
    }

    /// Read data from TCP stream.
    fn read_tcp_data(&mut self) -> Result<usize> {
        if let Some(ref mut client) = self.tcp_client {
            let mut temp_buf = [0u8; 1024];
            match client.recv(&mut temp_buf) {
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

    /// Process packets from the buffer.
    fn process_packets(&mut self) -> usize {
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

            if self.read_buffer.len() < CYTON_PACKET_SIZE {
                break;
            }

            if let Some(packet) =
                OpenBciPacket::parse_cyton(&self.read_buffer[..CYTON_PACKET_SIZE], self.gain)
            {
                let marker = self.marker_queue.pop_front().unwrap_or(0.0);

                self.sample_count += 1;
                let timestamp = self.sample_count as f64 / Self::SAMPLING_RATE as f64;

                let sample = packet.to_sample(timestamp, marker);

                if let Some(ref buffer) = self.buffers[0] {
                    buffer.push(&sample);
                }

                if let Some(ref buffer) = self.buffers[1] {
                    let aux_sample = alloc::vec![
                        packet.sample_number as f64,
                        packet.aux_channels.get(0).copied().unwrap_or(0.0),
                        packet.aux_channels.get(1).copied().unwrap_or(0.0),
                        packet.aux_channels.get(2).copied().unwrap_or(0.0),
                    ];
                    buffer.push(&aux_sample);
                }

                packets_processed += 1;
            }

            self.read_buffer.drain(..CYTON_PACKET_SIZE);
        }

        packets_processed
    }

    /// Process incoming data.
    pub fn process(&mut self) -> Result<usize> {
        if !self.running.load(Ordering::Acquire) {
            return Ok(0);
        }

        self.read_tcp_data()?;
        Ok(self.process_packets())
    }
}

impl Board for CytonWifiBoard {
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
        // 1. Create HTTP client
        // 2. Connect to WiFi shield
        // 3. Verify connection with /board endpoint

        self.state = BoardState::Ready;
        Ok(())
    }

    fn release_session(&mut self) -> Result<()> {
        if self.state == BoardState::Streaming {
            self.stop_stream()?;
        }

        if let Some(ref mut client) = self.tcp_client {
            let _ = client.close();
        }
        self.tcp_client = None;
        self.http_client = None;

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

        // Create ring buffers
        for (i, desc) in self.descriptions.iter().enumerate() {
            if desc.num_channels > 0 {
                self.buffers[i] = Some(RingBuffer::new(buffer_size, desc.num_channels));
            }
        }

        // In a real implementation:
        // 1. Send /stream/start HTTP command
        // 2. Connect to TCP stream port

        self.running.store(true, Ordering::Release);
        self.state = BoardState::Streaming;

        Ok(())
    }

    fn stop_stream(&mut self) -> Result<()> {
        if self.state != BoardState::Streaming {
            return Err(Error::new(ErrorCode::StreamThreadNotRunning));
        }

        // In a real implementation:
        // 1. Send /stream/stop HTTP command

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
        // WiFi board config commands
        match config {
            "stream_start" => {
                self.send_http_command("/stream/start")
            }
            "stream_stop" => {
                self.send_http_command("/stream/stop")
            }
            _ => {
                // Send as raw command
                #[cfg(feature = "std")]
                {
                    use std::format;
                    self.send_http_command(&format!("/command?command={}", config))
                }
                #[cfg(not(feature = "std"))]
                {
                    Err(Error::new(ErrorCode::GeneralError))
                }
            }
        }
    }
}

/// OpenBCI Cyton+Daisy WiFi board (16 channels).
pub struct CytonDaisyWifiBoard {
    inner: CytonWifiBoard,
}

impl CytonDaisyWifiBoard {
    /// Number of EEG channels.
    pub const NUM_EEG_CHANNELS: usize = 16;

    /// Create a new Cyton+Daisy WiFi board.
    pub fn new(ip_address: &str, ip_port: Option<u16>) -> Self {
        let mut inner = CytonWifiBoard::new(ip_address, ip_port);
        inner.descriptions[0] = Self::create_default_description();

        Self { inner }
    }

    fn create_default_description() -> BoardDescription {
        let num_channels = 22; // package + 16 EEG + 3 accel + timestamp + marker

        BoardDescription {
            sampling_rate: CytonWifiBoard::SAMPLING_RATE,
            num_channels,
            eeg_channels: (1..=Self::NUM_EEG_CHANNELS).collect(),
            emg_channels: Vec::new(),
            ecg_channels: Vec::new(),
            eog_channels: Vec::new(),
            ppg_channels: Vec::new(),
            eda_channels: Vec::new(),
            accel_channels: alloc::vec![17, 18, 19],
            gyro_channels: Vec::new(),
            temperature_channels: Vec::new(),
            battery_channel: Some(21),
            timestamp_channel: Some(19),
            marker_channel: Some(20),
            package_num_channel: Some(0),
            name: String::from("OpenBCI Cyton+Daisy WiFi"),
        }
    }
}

impl Board for CytonDaisyWifiBoard {
    fn state(&self) -> BoardState {
        self.inner.state()
    }

    fn description(&self, preset: Preset) -> &BoardDescription {
        self.inner.description(preset)
    }

    fn prepare_session(&mut self) -> Result<()> {
        self.inner.prepare_session()
    }

    fn release_session(&mut self) -> Result<()> {
        self.inner.release_session()
    }

    fn start_stream(&mut self, buffer_size: usize) -> Result<()> {
        self.inner.start_stream(buffer_size)
    }

    fn stop_stream(&mut self) -> Result<()> {
        self.inner.stop_stream()
    }

    fn get_current_board_data(&self, num_samples: usize, preset: Preset) -> Result<Vec<f64>> {
        self.inner.get_current_board_data(num_samples, preset)
    }

    fn get_board_data(&self, num_samples: Option<usize>, preset: Preset) -> Result<Vec<f64>> {
        self.inner.get_board_data(num_samples, preset)
    }

    fn get_board_data_count(&self, preset: Preset) -> Result<usize> {
        self.inner.get_board_data_count(preset)
    }

    fn insert_marker(&mut self, value: f64, preset: Preset) -> Result<()> {
        self.inner.insert_marker(value, preset)
    }

    fn config_board(&mut self, config: &str) -> Result<String> {
        self.inner.config_board(config)
    }
}

/// OpenBCI Ganglion WiFi board (4 channels).
pub struct GanglionWifiBoard {
    state: BoardState,
    descriptions: [BoardDescription; 3],
    buffers: [Option<RingBuffer<f64>>; 3],
    marker_queue: VecDeque<f64>,
    http_client: Option<Box<dyn HttpClient>>,
    tcp_client: Option<Box<dyn TcpClient>>,
    ip_address: String,
    ip_port: u16,
    running: AtomicBool,
    sample_count: u64,
}

impl GanglionWifiBoard {
    /// Ganglion sampling rate.
    pub const SAMPLING_RATE: u32 = 200;

    /// Number of EEG channels.
    pub const NUM_EEG_CHANNELS: usize = 4;

    /// Create a new Ganglion WiFi board.
    pub fn new(ip_address: &str, ip_port: Option<u16>) -> Self {
        Self {
            state: BoardState::NotReady,
            descriptions: [
                Self::create_default_description(),
                BoardDescription::default(),
                BoardDescription::default(),
            ],
            buffers: [None, None, None],
            marker_queue: VecDeque::new(),
            http_client: None,
            tcp_client: None,
            ip_address: String::from(ip_address),
            ip_port: ip_port.unwrap_or(WIFI_DEFAULT_PORT),
            running: AtomicBool::new(false),
            sample_count: 0,
        }
    }

    fn create_default_description() -> BoardDescription {
        let num_channels = 9; // package + 4 EEG + 3 accel + timestamp

        BoardDescription {
            sampling_rate: Self::SAMPLING_RATE,
            num_channels,
            eeg_channels: (1..=Self::NUM_EEG_CHANNELS).collect(),
            emg_channels: Vec::new(),
            ecg_channels: Vec::new(),
            eog_channels: Vec::new(),
            ppg_channels: Vec::new(),
            eda_channels: Vec::new(),
            accel_channels: alloc::vec![5, 6, 7],
            gyro_channels: Vec::new(),
            temperature_channels: Vec::new(),
            battery_channel: None,
            timestamp_channel: Some(8),
            marker_channel: None,
            package_num_channel: Some(0),
            name: String::from("OpenBCI Ganglion WiFi"),
        }
    }

    fn preset_index(preset: Preset) -> usize {
        preset.to_raw() as usize
    }
}

impl Board for GanglionWifiBoard {
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

        if let Some(ref mut client) = self.tcp_client {
            let _ = client.close();
        }
        self.tcp_client = None;
        self.http_client = None;

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
        Ok(String::from("OK"))
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_cyton_wifi_creation() {
        let board = CytonWifiBoard::new("192.168.1.100", None);
        assert_eq!(board.state(), BoardState::NotReady);

        let desc = board.description(Preset::Default);
        assert_eq!(desc.sampling_rate, 250);
        assert_eq!(desc.eeg_channels.len(), 8);
    }

    #[test]
    fn test_cyton_daisy_wifi_creation() {
        let board = CytonDaisyWifiBoard::new("192.168.1.100", None);
        assert_eq!(board.state(), BoardState::NotReady);

        let desc = board.description(Preset::Default);
        assert_eq!(desc.eeg_channels.len(), 16);
    }

    #[test]
    fn test_ganglion_wifi_creation() {
        let board = GanglionWifiBoard::new("192.168.1.100", None);
        assert_eq!(board.state(), BoardState::NotReady);

        let desc = board.description(Preset::Default);
        assert_eq!(desc.sampling_rate, 200);
        assert_eq!(desc.eeg_channels.len(), 4);
    }

    #[test]
    fn test_cyton_wifi_lifecycle() {
        let mut board = CytonWifiBoard::new("192.168.1.100", None);

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
