//! Muse headband implementations.
//!
//! Supports Muse 2, Muse S, and Muse 2016 headbands via BLE.

use alloc::boxed::Box;
use alloc::collections::VecDeque;
use alloc::format;
use alloc::string::{String, ToString};
use alloc::vec;
use alloc::vec::Vec;
use core::sync::atomic::{AtomicBool, AtomicU64, Ordering};

use brainflow_io::ble::{characteristics, BleClient, GattCharacteristic};
use brainflow_io::RingBuffer;
use brainflow_sys::error::{Error, ErrorCode, Result};

use crate::board::{reshape_to_row_major, Board, BoardDescription, BoardState};
use crate::preset::Preset;

/// Muse EEG sampling rate in Hz.
pub const MUSE_SAMPLING_RATE: u32 = 256;

/// Muse accelerometer sampling rate in Hz.
pub const MUSE_ACCEL_RATE: u32 = 52;

/// Muse gyroscope sampling rate in Hz.
pub const MUSE_GYRO_RATE: u32 = 52;

/// Muse PPG sampling rate in Hz (Muse S and 2 only).
pub const MUSE_PPG_RATE: u32 = 64;

/// Number of EEG samples per packet.
const SAMPLES_PER_PACKET: usize = 12;

/// EEG scale factor (nanovolts per LSB to microvolts).
const EEG_SCALE: f64 = 1.0 / 256.0; // Approximate scale

/// Accelerometer scale (mg per LSB to g).
const ACCEL_SCALE: f64 = 0.0000625;

/// Gyroscope scale (degrees per second per LSB).
const GYRO_SCALE: f64 = 0.0074768;

/// Muse packet parser.
pub mod packet {
    use alloc::vec::Vec;

    /// Parse Muse EEG packet (20 bytes -> 12 samples).
    ///
    /// The packet contains 12 samples packed as:
    /// - First 2 bytes: packet counter (big-endian)
    /// - Remaining 18 bytes: 12 12-bit samples (packed)
    pub fn parse_eeg_packet(data: &[u8]) -> Option<(u16, Vec<i16>)> {
        if data.len() < 20 {
            return None;
        }

        // First 2 bytes are counter
        let counter = ((data[0] as u16) << 8) | (data[1] as u16);

        // Remaining bytes contain 12 packed 12-bit samples
        let mut samples = Vec::with_capacity(12);

        // Parse 12-bit samples (3 bytes = 2 samples)
        for i in 0..6 {
            let offset = 2 + i * 3;
            if offset + 2 >= data.len() {
                break;
            }
            // First sample: bits [7:0] of byte 0, bits [7:4] of byte 1
            let s1 = ((data[offset] as i16) << 4) | ((data[offset + 1] >> 4) as i16);
            // Second sample: bits [3:0] of byte 1, bits [7:0] of byte 2
            let s2 = (((data[offset + 1] & 0x0F) as i16) << 8) | (data[offset + 2] as i16);

            // Sign extend from 12-bit
            let s1 = if s1 & 0x800 != 0 {
                s1 | 0xF000u16 as i16
            } else {
                s1
            };
            let s2 = if s2 & 0x800 != 0 {
                s2 | 0xF000u16 as i16
            } else {
                s2
            };

            samples.push(s1);
            samples.push(s2);
        }

        Some((counter, samples))
    }

    /// Parse Muse accelerometer/gyro packet (20 bytes -> 3 samples x 3 axes).
    pub fn parse_imu_packet(data: &[u8]) -> Option<(u16, Vec<[f64; 3]>)> {
        if data.len() < 20 {
            return None;
        }

        let counter = ((data[0] as u16) << 8) | (data[1] as u16);

        let mut samples = Vec::with_capacity(3);

        // 3 samples, each with X, Y, Z (16-bit signed, big-endian)
        for i in 0..3 {
            let offset = 2 + i * 6;
            if offset + 5 >= data.len() {
                break;
            }
            let x = ((data[offset] as i16) << 8) | (data[offset + 1] as i16);
            let y = ((data[offset + 2] as i16) << 8) | (data[offset + 3] as i16);
            let z = ((data[offset + 4] as i16) << 8) | (data[offset + 5] as i16);
            samples.push([x as f64, y as f64, z as f64]);
        }

        Some((counter, samples))
    }

    /// Parse Muse PPG packet (6 PPG samples).
    pub fn parse_ppg_packet(data: &[u8]) -> Option<(u16, Vec<u32>)> {
        if data.len() < 20 {
            return None;
        }

        let counter = ((data[0] as u16) << 8) | (data[1] as u16);

        let mut samples = Vec::with_capacity(6);

        // 6 samples, each 24-bit unsigned
        for i in 0..6 {
            let offset = 2 + i * 3;
            if offset + 2 >= data.len() {
                break;
            }
            let value = ((data[offset] as u32) << 16)
                | ((data[offset + 1] as u32) << 8)
                | (data[offset + 2] as u32);
            samples.push(value);
        }

        Some((counter, samples))
    }
}

/// Muse headband variant.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum MuseVariant {
    /// Muse 2 (2018+, has PPG).
    Muse2,
    /// Muse S (2020+, has PPG).
    MuseS,
    /// Original Muse (2016).
    Muse2016,
}

impl MuseVariant {
    /// Check if this variant has PPG sensor.
    pub fn has_ppg(&self) -> bool {
        matches!(self, MuseVariant::Muse2 | MuseVariant::MuseS)
    }

    /// Get the number of EEG channels.
    pub fn num_eeg_channels(&self) -> usize {
        match self {
            MuseVariant::Muse2 | MuseVariant::MuseS => 5,
            MuseVariant::Muse2016 => 4,
        }
    }

    /// Get the device name.
    pub fn name(&self) -> &'static str {
        match self {
            MuseVariant::Muse2 => "Muse 2",
            MuseVariant::MuseS => "Muse S",
            MuseVariant::Muse2016 => "Muse 2016",
        }
    }
}

/// Muse board implementation.
pub struct MuseBoard {
    /// Board state.
    state: BoardState,
    /// Board variant.
    variant: MuseVariant,
    /// Board descriptions for each preset.
    descriptions: [BoardDescription; 3],
    /// Data buffers for each preset.
    buffers: [Option<RingBuffer<f64>>; 3],
    /// Marker queues for each preset.
    marker_queues: [VecDeque<f64>; 3],
    /// BLE client.
    ble_client: Option<Box<dyn BleClient>>,
    /// Device address/name.
    device_address: String,
    /// Running flag.
    running: AtomicBool,
    /// Sample counter.
    sample_count: AtomicU64,
    /// EEG characteristics.
    eeg_chars: Vec<GattCharacteristic>,
    /// Accelerometer characteristic.
    accel_char: Option<GattCharacteristic>,
    /// Gyroscope characteristic.
    gyro_char: Option<GattCharacteristic>,
    /// PPG characteristic.
    ppg_char: Option<GattCharacteristic>,
    /// Control characteristic.
    control_char: Option<GattCharacteristic>,
}

impl MuseBoard {
    /// Create a new Muse board.
    pub fn new(variant: MuseVariant) -> Self {
        let num_eeg = variant.num_eeg_channels();

        // Default preset: EEG data
        // Channels: [package_num, TP9, AF7, AF8, TP10, (AUX for Muse2/S), timestamp, marker]
        let default_num_channels = 1 + num_eeg + 2; // package + EEG + timestamp + marker

        let default_desc = BoardDescription {
            sampling_rate: MUSE_SAMPLING_RATE,
            num_channels: default_num_channels,
            package_num_channel: Some(0),
            eeg_channels: (1..=num_eeg).collect(),
            timestamp_channel: Some(default_num_channels - 2),
            marker_channel: Some(default_num_channels - 1),
            name: variant.name().to_string(),
            ..Default::default()
        };

        // Auxiliary preset: IMU data
        // Channels: [package_num, accel_x, accel_y, accel_z, gyro_x, gyro_y, gyro_z]
        let aux_desc = BoardDescription {
            sampling_rate: MUSE_ACCEL_RATE,
            num_channels: 7,
            package_num_channel: Some(0),
            accel_channels: vec![1, 2, 3],
            gyro_channels: vec![4, 5, 6],
            name: format!("{} IMU", variant.name()),
            ..Default::default()
        };

        // Ancillary preset: PPG data (for Muse 2/S only)
        let ancillary_desc = if variant.has_ppg() {
            BoardDescription {
                sampling_rate: MUSE_PPG_RATE,
                num_channels: 4, // package + ppg_ambient + ppg_ir + ppg_red
                package_num_channel: Some(0),
                ppg_channels: vec![1, 2, 3],
                name: format!("{} PPG", variant.name()),
                ..Default::default()
            }
        } else {
            BoardDescription::default()
        };

        Self {
            state: BoardState::NotReady,
            variant,
            descriptions: [default_desc, aux_desc, ancillary_desc],
            buffers: [None, None, None],
            marker_queues: [VecDeque::new(), VecDeque::new(), VecDeque::new()],
            ble_client: None,
            device_address: String::new(),
            running: AtomicBool::new(false),
            sample_count: AtomicU64::new(0),
            eeg_chars: Vec::new(),
            accel_char: None,
            gyro_char: None,
            ppg_char: None,
            control_char: None,
        }
    }

    /// Create Muse 2 board.
    pub fn muse2() -> Self {
        Self::new(MuseVariant::Muse2)
    }

    /// Create Muse S board.
    pub fn muse_s() -> Self {
        Self::new(MuseVariant::MuseS)
    }

    /// Create Muse 2016 board.
    pub fn muse_2016() -> Self {
        Self::new(MuseVariant::Muse2016)
    }

    /// Set BLE client (for dependency injection).
    pub fn set_ble_client(&mut self, client: Box<dyn BleClient>) {
        self.ble_client = Some(client);
    }

    /// Set device address.
    pub fn set_device_address(&mut self, address: String) {
        self.device_address = address;
    }

    fn preset_index(preset: Preset) -> usize {
        preset.to_raw() as usize
    }

    /// Send command to Muse.
    fn send_command(&mut self, command: &[u8]) -> Result<()> {
        if let (Some(client), Some(char)) = (&mut self.ble_client, &self.control_char) {
            client
                .write_characteristic(char, command, false)
                .map_err(|_| Error::new(ErrorCode::FailedToSendData))
        } else {
            Err(Error::new(ErrorCode::BoardNotReady))
        }
    }

    /// Start streaming command.
    fn start_streaming_cmd(&mut self) -> Result<()> {
        // Muse uses specific protocol commands
        // 'd' - start streaming
        self.send_command(b"d")
    }

    /// Stop streaming command.
    fn stop_streaming_cmd(&mut self) -> Result<()> {
        // 'h' - halt/stop streaming
        self.send_command(b"h")
    }

    /// Get current timestamp.
    #[cfg(feature = "std")]
    fn get_timestamp(&self) -> f64 {
        use std::time::{SystemTime, UNIX_EPOCH};
        SystemTime::now()
            .duration_since(UNIX_EPOCH)
            .map(|d| d.as_secs_f64())
            .unwrap_or(0.0)
    }

    #[cfg(not(feature = "std"))]
    fn get_timestamp(&self) -> f64 {
        0.0
    }
}

impl Default for MuseBoard {
    fn default() -> Self {
        Self::new(MuseVariant::Muse2)
    }
}

impl Board for MuseBoard {
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

        if self.device_address.is_empty() {
            return Err(Error::new(ErrorCode::InvalidArguments));
        }

        self.state = BoardState::Ready;
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

        // Connect to device
        if let Some(ref mut client) = self.ble_client {
            let timeout = 10000; // 10 second timeout
            client
                .connect(&self.device_address, timeout)
                .map_err(|_| Error::new(ErrorCode::UnableToOpenPort))?;

            // Discover services and characteristics
            let services = client
                .discover_services()
                .map_err(|_| Error::new(ErrorCode::BoardNotReady))?;

            for service in services {
                let chars = client
                    .discover_characteristics(&service)
                    .map_err(|_| Error::new(ErrorCode::BoardNotReady))?;

                for char in chars {
                    // Match Muse characteristics by UUID
                    if char.uuid == characteristics::MUSE_CONTROL {
                        self.control_char = Some(char.clone());
                    } else if char.uuid == characteristics::MUSE_TP9 {
                        self.eeg_chars.push(char.clone());
                    } else if char.uuid == characteristics::MUSE_AF7 {
                        self.eeg_chars.push(char.clone());
                    } else if char.uuid == characteristics::MUSE_AF8 {
                        self.eeg_chars.push(char.clone());
                    } else if char.uuid == characteristics::MUSE_TP10 {
                        self.eeg_chars.push(char.clone());
                    } else if char.uuid == characteristics::MUSE_RIGHTAUX {
                        if self.variant.num_eeg_channels() == 5 {
                            self.eeg_chars.push(char.clone());
                        }
                    } else if char.uuid == characteristics::MUSE_ACCEL {
                        self.accel_char = Some(char.clone());
                    } else if char.uuid == characteristics::MUSE_GYRO {
                        self.gyro_char = Some(char.clone());
                    } else if char.uuid == characteristics::MUSE_PPG && self.variant.has_ppg() {
                        self.ppg_char = Some(char.clone());
                    }
                }
            }

            // Start streaming
            self.start_streaming_cmd()?;
        } else {
            return Err(Error::new(ErrorCode::BoardNotReady));
        }

        self.running.store(true, Ordering::SeqCst);
        self.state = BoardState::Streaming;
        Ok(())
    }

    fn stop_stream(&mut self) -> Result<()> {
        if self.state != BoardState::Streaming {
            return Err(Error::new(ErrorCode::StreamThreadNotRunning));
        }

        let _ = self.stop_streaming_cmd();
        self.running.store(false, Ordering::SeqCst);
        self.state = BoardState::Ready;
        Ok(())
    }

    fn release_session(&mut self) -> Result<()> {
        if self.state == BoardState::Streaming {
            self.stop_stream()?;
        }

        if let Some(ref mut client) = self.ble_client {
            let _ = client.disconnect();
        }

        self.buffers = [None, None, None];
        self.marker_queues = [VecDeque::new(), VecDeque::new(), VecDeque::new()];
        self.eeg_chars.clear();
        self.accel_char = None;
        self.gyro_char = None;
        self.ppg_char = None;
        self.control_char = None;
        self.state = BoardState::NotReady;
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

    fn config_board(&mut self, config: &str) -> Result<String> {
        // Send raw command to Muse
        self.send_command(config.as_bytes())?;
        Ok(String::new())
    }
}

/// Muse 2 board (5 EEG channels + PPG).
pub type Muse2Board = MuseBoard;

/// Muse S board (5 EEG channels + PPG).
pub type MuseSBoard = MuseBoard;

/// Muse 2016 board (4 EEG channels).
pub type Muse2016Board = MuseBoard;

// Helper function to format name with IMU suffix
fn format(base: &str, suffix: &str) -> String {
    let mut s = String::from(base);
    s.push_str(suffix);
    s
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_muse_variant() {
        assert!(MuseVariant::Muse2.has_ppg());
        assert!(MuseVariant::MuseS.has_ppg());
        assert!(!MuseVariant::Muse2016.has_ppg());

        assert_eq!(MuseVariant::Muse2.num_eeg_channels(), 5);
        assert_eq!(MuseVariant::Muse2016.num_eeg_channels(), 4);
    }

    #[test]
    fn test_parse_eeg_packet() {
        // Create test packet with counter 0x0102 and 12 samples
        let mut data = [0u8; 20];
        data[0] = 0x01;
        data[1] = 0x02;
        // Rest is zeros (12 zero samples)

        let result = packet::parse_eeg_packet(&data);
        assert!(result.is_some());
        let (counter, samples) = result.unwrap();
        assert_eq!(counter, 0x0102);
        assert_eq!(samples.len(), 12);
    }

    #[test]
    fn test_parse_imu_packet() {
        let mut data = [0u8; 20];
        data[0] = 0x00;
        data[1] = 0x42;

        let result = packet::parse_imu_packet(&data);
        assert!(result.is_some());
        let (counter, samples) = result.unwrap();
        assert_eq!(counter, 0x0042);
        assert_eq!(samples.len(), 3);
    }

    #[test]
    fn test_muse_board_description() {
        let board = MuseBoard::muse2();
        let desc = board.description(Preset::Default);

        assert_eq!(desc.sampling_rate, MUSE_SAMPLING_RATE);
        assert_eq!(desc.eeg_channels.len(), 5);
    }

    #[test]
    fn test_muse_board_state() {
        let board = MuseBoard::muse2();
        assert_eq!(board.state(), BoardState::NotReady);
    }
}
