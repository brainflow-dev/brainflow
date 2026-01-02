//! OpenBCI Cyton board implementation.
//!
//! The Cyton is an 8-channel EEG board using the ADS1299 chip.
//! It communicates over serial at 115200 baud.

use alloc::boxed::Box;
use alloc::collections::VecDeque;
use alloc::string::String;
use alloc::vec::Vec;
use core::sync::atomic::{AtomicBool, Ordering};

use brainflow_io::{IoError, RingBuffer, SerialConfig, SerialPort};
use brainflow_sys::error::{Error, ErrorCode, Result};

use crate::board::{Board, BoardDescription, BoardState, reshape_to_row_major};
use crate::preset::Preset;

use super::packet::{commands, OpenBciPacket, CYTON_PACKET_SIZE, HEADER_BYTE};

/// OpenBCI Cyton board (8-channel EEG).
pub struct CytonBoard {
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
    packet_buffer: Vec<u8>,
}

impl CytonBoard {
    /// Cyton sampling rate.
    pub const SAMPLING_RATE: u32 = 250;

    /// Number of EEG channels.
    pub const NUM_EEG_CHANNELS: usize = 8;

    /// Default gain setting.
    pub const DEFAULT_GAIN: f64 = 24.0;

    /// Create a new Cyton board.
    ///
    /// # Arguments
    /// * `port_name` - Serial port name (e.g., "/dev/ttyUSB0", "COM3")
    pub fn new(port_name: &str) -> Self {
        Self {
            state: BoardState::NotReady,
            descriptions: [
                Self::create_default_description(),
                Self::create_auxiliary_description(),
                BoardDescription::default(),
            ],
            buffers: [None, None, None],
            marker_queue: VecDeque::new(),
            serial_port: None,
            port_name: String::from(port_name),
            gain: Self::DEFAULT_GAIN,
            running: AtomicBool::new(false),
            sample_count: 0,
            read_buffer: Vec::with_capacity(4096),
            packet_buffer: Vec::with_capacity(CYTON_PACKET_SIZE),
        }
    }

    /// Set the channel gain.
    pub fn set_gain(&mut self, gain: f64) {
        self.gain = gain;
    }

    fn create_default_description() -> BoardDescription {
        // Channels: package_num, 8 EEG, 3 accel, timestamp, marker, battery = 14
        let num_channels = 14;

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
            name: String::from("OpenBCI Cyton"),
        }
    }

    fn create_auxiliary_description() -> BoardDescription {
        // Auxiliary preset with just accelerometer data
        BoardDescription {
            sampling_rate: Self::SAMPLING_RATE,
            num_channels: 4, // package + 3 accel
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
            name: String::from("OpenBCI Cyton Aux"),
        }
    }

    fn preset_index(preset: Preset) -> usize {
        preset.to_raw() as usize
    }

    /// Send a command to the board.
    fn send_command(&mut self, cmd: u8) -> Result<()> {
        if let Some(ref mut port) = self.serial_port {
            port.write(&[cmd]).map_err(|_| Error::new(ErrorCode::FailedToSendData))?;
            port.flush().map_err(|_| Error::new(ErrorCode::FailedToSendData))?;
            Ok(())
        } else {
            Err(Error::new(ErrorCode::BoardNotReady))
        }
    }

    /// Send a string command to the board.
    fn send_string(&mut self, cmd: &str) -> Result<()> {
        if let Some(ref mut port) = self.serial_port {
            port.write(cmd.as_bytes()).map_err(|_| Error::new(ErrorCode::FailedToSendData))?;
            port.flush().map_err(|_| Error::new(ErrorCode::FailedToSendData))?;
            Ok(())
        } else {
            Err(Error::new(ErrorCode::BoardNotReady))
        }
    }

    /// Read available data from serial port.
    fn read_data(&mut self) -> Result<usize> {
        if let Some(ref mut port) = self.serial_port {
            let mut temp_buf = [0u8; 1024];
            match port.read(&mut temp_buf) {
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

    /// Parse packets from the read buffer and push to ring buffer.
    fn process_packets(&mut self) -> usize {
        let mut packets_processed = 0;

        loop {
            // Find packet header
            let header_pos = self.read_buffer.iter().position(|&b| b == HEADER_BYTE);

            match header_pos {
                Some(pos) if pos > 0 => {
                    // Discard bytes before header
                    self.read_buffer.drain(..pos);
                }
                None => {
                    // No header found, clear buffer
                    self.read_buffer.clear();
                    break;
                }
                _ => {}
            }

            // Check if we have a complete packet
            if self.read_buffer.len() < CYTON_PACKET_SIZE {
                break;
            }

            // Try to parse packet
            if let Some(packet) = OpenBciPacket::parse_cyton(&self.read_buffer[..CYTON_PACKET_SIZE], self.gain) {
                // Get marker if available
                let marker = self.marker_queue.pop_front().unwrap_or(0.0);

                // Calculate timestamp
                self.sample_count += 1;
                let timestamp = self.sample_count as f64 / Self::SAMPLING_RATE as f64;

                // Convert to sample vector
                let sample = packet.to_sample(timestamp, marker);

                // Push to default preset buffer
                if let Some(ref buffer) = self.buffers[0] {
                    buffer.push(&sample);
                }

                // Also push aux data to auxiliary preset buffer
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

            // Remove processed bytes
            self.read_buffer.drain(..CYTON_PACKET_SIZE);
        }

        packets_processed
    }

    /// Process data in streaming mode.
    /// This should be called periodically (e.g., in a loop or from a timer).
    pub fn process(&mut self) -> Result<usize> {
        if !self.running.load(Ordering::Acquire) {
            return Ok(0);
        }

        self.read_data()?;
        Ok(self.process_packets())
    }
}

impl Board for CytonBoard {
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

        // Note: In a real implementation, we would create the serial port here
        // For now, we'll need the caller to provide a serial port implementation
        // or use a factory pattern

        // Platform-specific serial port would be created here
        // For testing without hardware, we can still prepare the session

        self.state = BoardState::Ready;
        Ok(())
    }

    fn release_session(&mut self) -> Result<()> {
        if self.state == BoardState::Streaming {
            self.stop_stream()?;
        }

        // Close serial port
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

        // Create ring buffers
        for (i, desc) in self.descriptions.iter().enumerate() {
            if desc.num_channels > 0 {
                self.buffers[i] = Some(RingBuffer::new(buffer_size, desc.num_channels));
            }
        }

        // Send start streaming command
        if self.serial_port.is_some() {
            self.send_command(commands::START_STREAMING)?;
        }

        self.running.store(true, Ordering::Release);
        self.state = BoardState::Streaming;

        Ok(())
    }

    fn stop_stream(&mut self) -> Result<()> {
        if self.state != BoardState::Streaming {
            return Err(Error::new(ErrorCode::StreamThreadNotRunning));
        }

        // Send stop streaming command
        if self.serial_port.is_some() {
            self.send_command(commands::STOP_STREAMING)?;
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
        // Parse and handle configuration commands
        match config {
            "reset" | "v" => {
                self.send_command(commands::SOFT_RESET)?;
                Ok(String::from("Reset sent"))
            }
            "start" | "b" => {
                self.send_command(commands::START_STREAMING)?;
                Ok(String::from("Streaming started"))
            }
            "stop" | "s" => {
                self.send_command(commands::STOP_STREAMING)?;
                Ok(String::from("Streaming stopped"))
            }
            "default" | "d" => {
                self.send_command(commands::DEFAULT_CHANNEL_SETTINGS)?;
                Ok(String::from("Default settings applied"))
            }
            _ => {
                // Send raw command string
                self.send_string(config)?;
                Ok(String::from("Command sent"))
            }
        }
    }
}

/// OpenBCI Cyton + Daisy board (16-channel EEG).
///
/// The Daisy module adds 8 additional channels to the Cyton.
/// Data is transmitted in alternating packets.
pub struct CytonDaisyBoard {
    inner: CytonBoard,
    daisy_buffer: Vec<u8>,
    last_sample_number: u8,
    combined_eeg: Vec<f64>,
}

impl CytonDaisyBoard {
    /// Number of EEG channels (8 Cyton + 8 Daisy).
    pub const NUM_EEG_CHANNELS: usize = 16;

    /// Create a new Cyton + Daisy board.
    pub fn new(port_name: &str) -> Self {
        let mut inner = CytonBoard::new(port_name);
        inner.descriptions[0] = Self::create_default_description();

        Self {
            inner,
            daisy_buffer: Vec::with_capacity(CYTON_PACKET_SIZE),
            last_sample_number: 0,
            combined_eeg: Vec::with_capacity(Self::NUM_EEG_CHANNELS),
        }
    }

    fn create_default_description() -> BoardDescription {
        // Channels: package_num, 16 EEG, 3 accel, timestamp, marker, battery = 22
        let num_channels = 22;

        BoardDescription {
            sampling_rate: CytonBoard::SAMPLING_RATE,
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
            name: String::from("OpenBCI Cyton + Daisy"),
        }
    }
}

impl Board for CytonDaisyBoard {
    fn state(&self) -> BoardState {
        self.inner.state()
    }

    fn description(&self, preset: Preset) -> &BoardDescription {
        self.inner.description(preset)
    }

    fn prepare_session(&mut self) -> Result<()> {
        self.inner.prepare_session()?;

        // Attach Daisy module
        if self.inner.serial_port.is_some() {
            self.inner.send_command(commands::ATTACH_DAISY)?;
        }

        Ok(())
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

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_cyton_board_creation() {
        let board = CytonBoard::new("/dev/ttyUSB0");
        assert_eq!(board.state(), BoardState::NotReady);

        let desc = board.description(Preset::Default);
        assert_eq!(desc.sampling_rate, 250);
        assert_eq!(desc.eeg_channels.len(), 8);
        assert_eq!(desc.accel_channels.len(), 3);
    }

    #[test]
    fn test_cyton_daisy_board_creation() {
        let board = CytonDaisyBoard::new("/dev/ttyUSB0");
        assert_eq!(board.state(), BoardState::NotReady);

        let desc = board.description(Preset::Default);
        assert_eq!(desc.eeg_channels.len(), 16);
    }

    #[test]
    fn test_cyton_lifecycle_without_hardware() {
        let mut board = CytonBoard::new("/dev/ttyUSB0");

        // Can prepare session without actual hardware
        board.prepare_session().unwrap();
        assert_eq!(board.state(), BoardState::Ready);

        // Can start streaming
        board.start_stream(1000).unwrap();
        assert_eq!(board.state(), BoardState::Streaming);

        // Insert marker
        board.insert_marker(42.0, Preset::Default).unwrap();

        // Stop streaming
        board.stop_stream().unwrap();
        assert_eq!(board.state(), BoardState::Ready);

        // Release session
        board.release_session().unwrap();
        assert_eq!(board.state(), BoardState::NotReady);
    }
}
