//! OpenBCI Ganglion board implementation.
//!
//! The Ganglion is a 4-channel biosignal acquisition board that communicates
//! over Bluetooth Low Energy (BLE).

use alloc::boxed::Box;
use alloc::collections::VecDeque;
use alloc::string::{String, ToString};
use alloc::vec;
use alloc::vec::Vec;
use core::sync::atomic::{AtomicBool, AtomicU64, Ordering};

use brainflow_io::ble::{characteristics, BleClient, GattCharacteristic};
use brainflow_io::RingBuffer;
use brainflow_sys::error::{Error, ErrorCode, Result};

use crate::board::{reshape_to_row_major, Board, BoardDescription, BoardState};
use crate::preset::Preset;

/// Ganglion sampling rate in Hz.
pub const GANGLION_SAMPLING_RATE: u32 = 200;

/// Number of EEG channels.
pub const GANGLION_EEG_CHANNELS: usize = 4;

/// Number of accelerometer channels.
pub const GANGLION_ACCEL_CHANNELS: usize = 3;

/// EEG scale factor (microvolts per LSB).
/// MCP3912 with 1.2V Vref, 24-bit, gain=1: 1.2V / 2^23 * 1e6 = 0.143 μV/LSB
pub const GANGLION_EEG_SCALE: f64 = 1.2 / 8388607.0 * 1_000_000.0;

/// Accelerometer scale (g per LSB for MMA8452Q ±2g range).
pub const GANGLION_ACCEL_SCALE: f64 = 0.001;

/// Ganglion packet types based on first byte.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum GanglionPacketType {
    /// Uncompressed 19-bit data (bytes 0-100).
    Uncompressed19,
    /// Compressed 18-bit data with accelerometer (bytes 101-200).
    Compressed18WithAccel,
    /// Compressed 19-bit data (bytes 201-205).
    Compressed19Delta1,
    /// Compressed 19-bit data (bytes 206).
    Compressed19Delta2,
    /// Impedance channel 1.
    ImpedanceChannel1,
    /// Impedance channel 2.
    ImpedanceChannel2,
    /// Impedance channel 3.
    ImpedanceChannel3,
    /// Impedance channel 4.
    ImpedanceChannel4,
    /// Impedance reference.
    ImpedanceReference,
    /// Unknown/invalid.
    Unknown(u8),
}

impl From<u8> for GanglionPacketType {
    fn from(byte: u8) -> Self {
        match byte {
            0..=100 => GanglionPacketType::Uncompressed19,
            101..=200 => GanglionPacketType::Compressed18WithAccel,
            201..=205 => GanglionPacketType::Compressed19Delta1,
            206 => GanglionPacketType::Compressed19Delta2,
            207 => GanglionPacketType::ImpedanceChannel1,
            208 => GanglionPacketType::ImpedanceChannel2,
            209 => GanglionPacketType::ImpedanceChannel3,
            210 => GanglionPacketType::ImpedanceChannel4,
            211 => GanglionPacketType::ImpedanceReference,
            other => GanglionPacketType::Unknown(other),
        }
    }
}

/// Parsed Ganglion packet.
#[derive(Debug, Clone)]
pub struct GanglionPacket {
    /// Sample number.
    pub sample_number: u8,
    /// Packet type.
    pub packet_type: GanglionPacketType,
    /// 4-channel EEG data.
    pub eeg_channels: [i32; 4],
    /// Optional accelerometer data.
    pub accel: Option<[i16; 3]>,
    /// Optional impedance value.
    pub impedance: Option<(usize, f64)>,
}

impl GanglionPacket {
    /// Parse a Ganglion BLE packet (20 bytes).
    pub fn parse(data: &[u8], last_values: &[i32; 4]) -> Option<Self> {
        if data.len() < 20 {
            return None;
        }

        let sample_number = data[0];
        let packet_type = GanglionPacketType::from(sample_number);

        match packet_type {
            GanglionPacketType::Uncompressed19 => {
                // 19-bit uncompressed: 4 channels * 19 bits = 76 bits = 9.5 bytes
                // Packed in bytes 1-19
                let eeg_channels = parse_uncompressed_19bit(&data[1..]);
                Some(Self {
                    sample_number,
                    packet_type,
                    eeg_channels,
                    accel: None,
                    impedance: None,
                })
            }
            GanglionPacketType::Compressed18WithAccel => {
                // 18-bit compressed deltas with accelerometer
                let (deltas, accel) = parse_compressed_18bit_accel(&data[1..]);
                let eeg_channels = [
                    last_values[0].wrapping_add(deltas[0]),
                    last_values[1].wrapping_add(deltas[1]),
                    last_values[2].wrapping_add(deltas[2]),
                    last_values[3].wrapping_add(deltas[3]),
                ];
                Some(Self {
                    sample_number,
                    packet_type,
                    eeg_channels,
                    accel: Some(accel),
                    impedance: None,
                })
            }
            GanglionPacketType::Compressed19Delta1 | GanglionPacketType::Compressed19Delta2 => {
                // 19-bit compressed deltas
                let deltas = parse_compressed_19bit(&data[1..]);
                let eeg_channels = [
                    last_values[0].wrapping_add(deltas[0]),
                    last_values[1].wrapping_add(deltas[1]),
                    last_values[2].wrapping_add(deltas[2]),
                    last_values[3].wrapping_add(deltas[3]),
                ];
                Some(Self {
                    sample_number,
                    packet_type,
                    eeg_channels,
                    accel: None,
                    impedance: None,
                })
            }
            GanglionPacketType::ImpedanceChannel1
            | GanglionPacketType::ImpedanceChannel2
            | GanglionPacketType::ImpedanceChannel3
            | GanglionPacketType::ImpedanceChannel4
            | GanglionPacketType::ImpedanceReference => {
                let channel = match packet_type {
                    GanglionPacketType::ImpedanceChannel1 => 0,
                    GanglionPacketType::ImpedanceChannel2 => 1,
                    GanglionPacketType::ImpedanceChannel3 => 2,
                    GanglionPacketType::ImpedanceChannel4 => 3,
                    GanglionPacketType::ImpedanceReference => 4,
                    _ => 0,
                };
                let impedance = parse_impedance(&data[1..]);
                Some(Self {
                    sample_number,
                    packet_type,
                    eeg_channels: *last_values,
                    accel: None,
                    impedance: Some((channel, impedance)),
                })
            }
            GanglionPacketType::Unknown(_) => None,
        }
    }
}

/// Parse 19-bit uncompressed data (4 channels).
fn parse_uncompressed_19bit(data: &[u8]) -> [i32; 4] {
    // 4 channels * 19 bits = 76 bits packed into bytes
    let mut channels = [0i32; 4];

    if data.len() < 10 {
        return channels;
    }

    // Channel 1: bits 0-18 (bytes 0-2, partial byte 3)
    let raw1 = ((data[0] as u32) << 11) | ((data[1] as u32) << 3) | ((data[2] as u32) >> 5);
    channels[0] = sign_extend_19(raw1 & 0x7FFFF);

    // Channel 2: bits 19-37
    let raw2 =
        (((data[2] & 0x1F) as u32) << 14) | ((data[3] as u32) << 6) | ((data[4] as u32) >> 2);
    channels[1] = sign_extend_19(raw2 & 0x7FFFF);

    // Channel 3: bits 38-56
    let raw3 = (((data[4] & 0x03) as u32) << 17)
        | ((data[5] as u32) << 9)
        | ((data[6] as u32) << 1)
        | ((data[7] as u32) >> 7);
    channels[2] = sign_extend_19(raw3 & 0x7FFFF);

    // Channel 4: bits 57-75
    let raw4 =
        (((data[7] & 0x7F) as u32) << 12) | ((data[8] as u32) << 4) | ((data[9] as u32) >> 4);
    channels[3] = sign_extend_19(raw4 & 0x7FFFF);

    channels
}

/// Parse 18-bit compressed deltas with accelerometer.
fn parse_compressed_18bit_accel(data: &[u8]) -> ([i32; 4], [i16; 3]) {
    let mut deltas = [0i32; 4];
    let mut accel = [0i16; 3];

    if data.len() < 19 {
        return (deltas, accel);
    }

    // First 4 deltas: 18 bits each = 72 bits = 9 bytes
    let raw1 = ((data[0] as u32) << 10) | ((data[1] as u32) << 2) | ((data[2] as u32) >> 6);
    deltas[0] = sign_extend_18(raw1 & 0x3FFFF);

    let raw2 =
        (((data[2] & 0x3F) as u32) << 12) | ((data[3] as u32) << 4) | ((data[4] as u32) >> 4);
    deltas[1] = sign_extend_18(raw2 & 0x3FFFF);

    let raw3 =
        (((data[4] & 0x0F) as u32) << 14) | ((data[5] as u32) << 6) | ((data[6] as u32) >> 2);
    deltas[2] = sign_extend_18(raw3 & 0x3FFFF);

    let raw4 = (((data[6] & 0x03) as u32) << 16) | ((data[7] as u32) << 8) | (data[8] as u32);
    deltas[3] = sign_extend_18(raw4 & 0x3FFFF);

    // Next 4 deltas would follow, then accelerometer at bytes 18-19
    // Accelerometer: 3 axes, 8-bit signed each
    if data.len() >= 12 {
        accel[0] = data[9] as i8 as i16;
        accel[1] = data[10] as i8 as i16;
        accel[2] = data[11] as i8 as i16;
    }

    (deltas, accel)
}

/// Parse 19-bit compressed deltas.
fn parse_compressed_19bit(data: &[u8]) -> [i32; 4] {
    let mut deltas = [0i32; 4];

    if data.len() < 10 {
        return deltas;
    }

    // Same format as uncompressed but values are deltas
    let raw1 = ((data[0] as u32) << 11) | ((data[1] as u32) << 3) | ((data[2] as u32) >> 5);
    deltas[0] = sign_extend_19(raw1 & 0x7FFFF);

    let raw2 =
        (((data[2] & 0x1F) as u32) << 14) | ((data[3] as u32) << 6) | ((data[4] as u32) >> 2);
    deltas[1] = sign_extend_19(raw2 & 0x7FFFF);

    let raw3 = (((data[4] & 0x03) as u32) << 17)
        | ((data[5] as u32) << 9)
        | ((data[6] as u32) << 1)
        | ((data[7] as u32) >> 7);
    deltas[2] = sign_extend_19(raw3 & 0x7FFFF);

    let raw4 =
        (((data[7] & 0x7F) as u32) << 12) | ((data[8] as u32) << 4) | ((data[9] as u32) >> 4);
    deltas[3] = sign_extend_19(raw4 & 0x7FFFF);

    deltas
}

/// Parse impedance value.
fn parse_impedance(data: &[u8]) -> f64 {
    // Impedance is encoded as ASCII in the packet
    let text: String = data
        .iter()
        .take_while(|&&b| b != 0 && (b.is_ascii_digit() || b == b'.'))
        .map(|&b| b as char)
        .collect();

    text.parse().unwrap_or(0.0)
}

/// Sign-extend 19-bit value to 32-bit.
fn sign_extend_19(value: u32) -> i32 {
    if value & 0x40000 != 0 {
        (value | 0xFFF80000) as i32
    } else {
        value as i32
    }
}

/// Sign-extend 18-bit value to 32-bit.
fn sign_extend_18(value: u32) -> i32 {
    if value & 0x20000 != 0 {
        (value | 0xFFFC0000) as i32
    } else {
        value as i32
    }
}

/// OpenBCI Ganglion board.
pub struct GanglionBoard {
    /// Board state.
    state: BoardState,
    /// Board descriptions for each preset.
    descriptions: [BoardDescription; 3],
    /// Data buffers for each preset.
    buffers: [Option<RingBuffer<f64>>; 3],
    /// Marker queues for each preset.
    marker_queues: [VecDeque<f64>; 3],
    /// BLE client.
    ble_client: Option<Box<dyn BleClient>>,
    /// Device address.
    device_address: String,
    /// Running flag.
    running: AtomicBool,
    /// Sample counter.
    sample_count: AtomicU64,
    /// Last sample values for delta decompression.
    last_values: [i32; 4],
    /// Receive characteristic.
    receive_char: Option<GattCharacteristic>,
    /// Send characteristic.
    send_char: Option<GattCharacteristic>,
    /// Enable accelerometer.
    accel_enabled: bool,
}

impl GanglionBoard {
    /// Create a new Ganglion board.
    pub fn new() -> Self {
        // Default preset: EEG + accel data
        // Channels: package_num, 4 EEG, 3 accel, timestamp, marker
        let default_num_channels = 10;

        let default_desc = BoardDescription {
            sampling_rate: GANGLION_SAMPLING_RATE,
            num_channels: default_num_channels,
            package_num_channel: Some(0),
            eeg_channels: vec![1, 2, 3, 4],
            accel_channels: vec![5, 6, 7],
            timestamp_channel: Some(8),
            marker_channel: Some(9),
            name: "Ganglion".to_string(),
            ..Default::default()
        };

        // Auxiliary preset: not used for Ganglion
        let aux_desc = BoardDescription::default();

        // Ancillary preset: not used for Ganglion
        let ancillary_desc = BoardDescription::default();

        Self {
            state: BoardState::NotReady,
            descriptions: [default_desc, aux_desc, ancillary_desc],
            buffers: [None, None, None],
            marker_queues: [VecDeque::new(), VecDeque::new(), VecDeque::new()],
            ble_client: None,
            device_address: String::new(),
            running: AtomicBool::new(false),
            sample_count: AtomicU64::new(0),
            last_values: [0; 4],
            receive_char: None,
            send_char: None,
            accel_enabled: true,
        }
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

    /// Send command to Ganglion.
    fn send_command(&mut self, command: u8) -> Result<()> {
        if let (Some(client), Some(char)) = (&mut self.ble_client, &self.send_char) {
            client
                .write_characteristic(char, &[command], false)
                .map_err(|_| Error::new(ErrorCode::FailedToSendData))
        } else {
            Err(Error::new(ErrorCode::BoardNotReady))
        }
    }

    /// Process a received BLE packet.
    pub fn process_packet(&mut self, data: &[u8]) {
        if let Some(packet) = GanglionPacket::parse(data, &self.last_values) {
            // Update last values
            self.last_values = packet.eeg_channels;

            // Build sample
            let mut sample = Vec::with_capacity(10);

            // Package number
            sample.push(packet.sample_number as f64);

            // EEG channels (convert to microvolts)
            for ch in &packet.eeg_channels {
                sample.push(*ch as f64 * GANGLION_EEG_SCALE);
            }

            // Accelerometer
            if let Some(accel) = packet.accel {
                sample.push(accel[0] as f64 * GANGLION_ACCEL_SCALE);
                sample.push(accel[1] as f64 * GANGLION_ACCEL_SCALE);
                sample.push(accel[2] as f64 * GANGLION_ACCEL_SCALE);
            } else {
                sample.push(0.0);
                sample.push(0.0);
                sample.push(0.0);
            }

            // Timestamp
            sample.push(self.get_timestamp());

            // Marker
            let marker = self.marker_queues[0].pop_front().unwrap_or(0.0);
            sample.push(marker);

            // Add to buffer
            if let Some(ref buffer) = self.buffers[0] {
                buffer.push(&sample);
            }

            self.sample_count.fetch_add(1, Ordering::Relaxed);
        }
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

impl Default for GanglionBoard {
    fn default() -> Self {
        Self::new()
    }
}

impl Board for GanglionBoard {
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
            let timeout = 10000;
            client
                .connect(&self.device_address, timeout)
                .map_err(|_| Error::new(ErrorCode::UnableToOpenPort))?;

            // Discover services
            let services = client
                .discover_services()
                .map_err(|_| Error::new(ErrorCode::BoardNotReady))?;

            for service in services {
                let chars = client
                    .discover_characteristics(&service)
                    .map_err(|_| Error::new(ErrorCode::BoardNotReady))?;

                for char in chars {
                    if char.uuid == characteristics::GANGLION_RECEIVE {
                        self.receive_char = Some(char.clone());
                    } else if char.uuid == characteristics::GANGLION_SEND {
                        self.send_char = Some(char.clone());
                    }
                }
            }

            // Subscribe to receive characteristic
            if let Some(ref receive_char) = self.receive_char {
                // In a real implementation, we'd set up a callback here
                let _ = client.subscribe(receive_char, Box::new(|_data| {
                    // This would be called when data is received
                }));
            }

            // Start streaming: send 'b' command
            self.send_command(b'b')?;
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

        // Stop streaming: send 's' command
        let _ = self.send_command(b's');

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
        self.receive_char = None;
        self.send_char = None;
        self.last_values = [0; 4];
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
        // Process config commands
        for c in config.bytes() {
            self.send_command(c)?;
        }
        Ok(String::new())
    }
}

/// Ganglion commands.
pub mod commands {
    /// Start streaming.
    pub const START_STREAMING: u8 = b'b';
    /// Stop streaming.
    pub const STOP_STREAMING: u8 = b's';
    /// Soft reset.
    pub const SOFT_RESET: u8 = b'v';
    /// Enable accelerometer.
    pub const ENABLE_ACCEL: u8 = b'n';
    /// Disable accelerometer.
    pub const DISABLE_ACCEL: u8 = b'N';
    /// Enable impedance check.
    pub const ENABLE_IMPEDANCE: u8 = b'z';
    /// Disable impedance check.
    pub const DISABLE_IMPEDANCE: u8 = b'Z';
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_packet_type_from_byte() {
        assert_eq!(
            GanglionPacketType::from(0),
            GanglionPacketType::Uncompressed19
        );
        assert_eq!(
            GanglionPacketType::from(100),
            GanglionPacketType::Uncompressed19
        );
        assert_eq!(
            GanglionPacketType::from(101),
            GanglionPacketType::Compressed18WithAccel
        );
        assert_eq!(
            GanglionPacketType::from(201),
            GanglionPacketType::Compressed19Delta1
        );
        assert_eq!(
            GanglionPacketType::from(207),
            GanglionPacketType::ImpedanceChannel1
        );
    }

    #[test]
    fn test_sign_extend_19() {
        // Positive value
        assert_eq!(sign_extend_19(0x00001), 1);
        // Maximum positive
        assert_eq!(sign_extend_19(0x3FFFF), 262143);
        // Negative value (-1)
        assert_eq!(sign_extend_19(0x7FFFF), -1);
        // Minimum negative
        assert_eq!(sign_extend_19(0x40000), -262144);
    }

    #[test]
    fn test_sign_extend_18() {
        // Positive value
        assert_eq!(sign_extend_18(0x00001), 1);
        // Maximum positive
        assert_eq!(sign_extend_18(0x1FFFF), 131071);
        // Negative value (-1)
        assert_eq!(sign_extend_18(0x3FFFF), -1);
    }

    #[test]
    fn test_ganglion_board_description() {
        let board = GanglionBoard::new();
        let desc = board.description(Preset::Default);

        assert_eq!(desc.sampling_rate, GANGLION_SAMPLING_RATE);
        assert_eq!(desc.eeg_channels.len(), 4);
        assert_eq!(desc.accel_channels.len(), 3);
    }

    #[test]
    fn test_ganglion_board_state() {
        let board = GanglionBoard::new();
        assert_eq!(board.state(), BoardState::NotReady);
    }
}
