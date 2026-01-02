//! OpenBCI packet parsing.
//!
//! Handles the binary protocol used by OpenBCI boards.

use alloc::vec::Vec;

/// OpenBCI packet header byte.
pub const HEADER_BYTE: u8 = 0xA0;

/// Standard packet footer byte (normal data).
pub const FOOTER_STANDARD: u8 = 0xC0;

/// Packet footer for accelerometer data.
pub const FOOTER_ACCEL: u8 = 0xC0;

/// Packet footer for raw aux data.
pub const FOOTER_RAW_AUX: u8 = 0xC1;

/// Packet footer for user-defined.
pub const FOOTER_USER_DEFINED: u8 = 0xC2;

/// Packet footer for time-stamped data.
pub const FOOTER_TIMESTAMP: u8 = 0xC3;

/// Packet footer indicating no sample.
pub const FOOTER_NO_SAMPLE: u8 = 0xC6;

/// Cyton packet size in bytes.
pub const CYTON_PACKET_SIZE: usize = 33;

/// Number of EEG channels in a Cyton packet.
pub const CYTON_EEG_CHANNELS: usize = 8;

/// Number of auxiliary channels in a Cyton packet.
pub const CYTON_AUX_CHANNELS: usize = 3;

/// EEG scale factor for Cyton: 4.5V / (2^23 - 1) / 24 * 1e6 μV
/// With default gain of 24x
pub const CYTON_EEG_SCALE: f64 = 4.5 / 8388607.0 / 24.0 * 1_000_000.0;

/// Accelerometer scale factor (±4g range, 16-bit)
pub const CYTON_ACCEL_SCALE: f64 = 0.002 / 16.0; // g per LSB

/// Type of OpenBCI packet based on footer byte.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum PacketType {
    /// Standard packet with accelerometer data.
    Standard,
    /// Packet with raw auxiliary data.
    RawAux,
    /// User-defined auxiliary data.
    UserDefined,
    /// Time-stamped packet.
    Timestamp,
    /// No sample (error or placeholder).
    NoSample,
    /// Unknown packet type.
    Unknown(u8),
}

impl From<u8> for PacketType {
    fn from(footer: u8) -> Self {
        match footer {
            FOOTER_STANDARD | FOOTER_ACCEL => PacketType::Standard,
            FOOTER_RAW_AUX => PacketType::RawAux,
            FOOTER_USER_DEFINED => PacketType::UserDefined,
            FOOTER_TIMESTAMP => PacketType::Timestamp,
            FOOTER_NO_SAMPLE => PacketType::NoSample,
            other => PacketType::Unknown(other),
        }
    }
}

/// Parsed OpenBCI packet.
#[derive(Debug, Clone)]
pub struct OpenBciPacket {
    /// Sample number (0-255, wraps around).
    pub sample_number: u8,
    /// EEG channel data in microvolts.
    pub eeg_channels: Vec<f64>,
    /// Auxiliary channel data (accelerometer or raw).
    pub aux_channels: Vec<f64>,
    /// Packet type based on footer.
    pub packet_type: PacketType,
}

impl OpenBciPacket {
    /// Parse a Cyton packet from raw bytes.
    ///
    /// # Arguments
    /// * `data` - 33-byte raw packet
    /// * `gain` - Gain setting for EEG channels (default 24)
    ///
    /// # Returns
    /// Parsed packet or None if invalid.
    pub fn parse_cyton(data: &[u8], gain: f64) -> Option<Self> {
        if data.len() < CYTON_PACKET_SIZE {
            return None;
        }

        // Validate header
        if data[0] != HEADER_BYTE {
            return None;
        }

        let sample_number = data[1];
        let packet_type = PacketType::from(data[CYTON_PACKET_SIZE - 1]);

        // Parse 8 EEG channels (24-bit signed integers, MSB first)
        let scale = 4.5 / 8388607.0 / gain * 1_000_000.0;
        let mut eeg_channels = Vec::with_capacity(CYTON_EEG_CHANNELS);
        for i in 0..CYTON_EEG_CHANNELS {
            let offset = 2 + i * 3;
            let raw = parse_int24(&data[offset..offset + 3]);
            eeg_channels.push(raw as f64 * scale);
        }

        // Parse 3 aux channels (16-bit signed integers, MSB first)
        let mut aux_channels = Vec::with_capacity(CYTON_AUX_CHANNELS);
        for i in 0..CYTON_AUX_CHANNELS {
            let offset = 26 + i * 2;
            let raw = parse_int16(&data[offset..offset + 2]);
            // Apply accelerometer scale for standard packets
            let value = match packet_type {
                PacketType::Standard => raw as f64 * CYTON_ACCEL_SCALE,
                _ => raw as f64,
            };
            aux_channels.push(value);
        }

        Some(Self {
            sample_number,
            eeg_channels,
            aux_channels,
            packet_type,
        })
    }

    /// Parse a CytonDaisy packet (combines two Cyton packets).
    ///
    /// Daisy mode sends alternating packets: odd sample numbers have channels 1-8,
    /// even sample numbers have channels 9-16.
    pub fn parse_daisy_pair(
        first: &[u8],
        second: &[u8],
        gain: f64,
    ) -> Option<(Self, Vec<f64>)> {
        let packet1 = Self::parse_cyton(first, gain)?;
        let packet2 = Self::parse_cyton(second, gain)?;

        // Daisy packets alternate: first has channels 1-8, second has 9-16
        let mut combined_eeg = packet1.eeg_channels.clone();
        combined_eeg.extend(&packet2.eeg_channels);

        Some((packet1, combined_eeg))
    }

    /// Convert packet to flat sample vector for buffer storage.
    ///
    /// Format: [package_num, eeg_ch1, ..., eeg_ch8, accel_x, accel_y, accel_z, timestamp, marker, battery]
    pub fn to_sample(&self, timestamp: f64, marker: f64) -> Vec<f64> {
        let mut sample = Vec::with_capacity(14);

        // Package number
        sample.push(self.sample_number as f64);

        // EEG channels
        sample.extend(&self.eeg_channels);

        // Aux/accel channels
        sample.extend(&self.aux_channels);

        // Timestamp
        sample.push(timestamp);

        // Marker
        sample.push(marker);

        // Battery (placeholder)
        sample.push(0.0);

        sample
    }
}

/// Parse 24-bit signed integer (MSB first).
fn parse_int24(data: &[u8]) -> i32 {
    if data.len() < 3 {
        return 0;
    }

    // Combine bytes (MSB first)
    let unsigned = ((data[0] as u32) << 16) | ((data[1] as u32) << 8) | (data[2] as u32);

    // Sign extend from 24-bit to 32-bit
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

/// OpenBCI commands for Cyton board.
pub mod commands {
    /// Start streaming command.
    pub const START_STREAMING: u8 = b'b';

    /// Stop streaming command.
    pub const STOP_STREAMING: u8 = b's';

    /// Soft reset command.
    pub const SOFT_RESET: u8 = b'v';

    /// Get firmware version.
    pub const GET_VERSION: u8 = b'V';

    /// Default channel settings.
    pub const DEFAULT_CHANNEL_SETTINGS: u8 = b'd';

    /// Enable accelerometer.
    pub const ENABLE_ACCEL: u8 = b'n';

    /// Disable accelerometer.
    pub const DISABLE_ACCEL: u8 = b'N';

    /// Attach Daisy module.
    pub const ATTACH_DAISY: u8 = b'C';

    /// Detach Daisy module.
    pub const DETACH_DAISY: u8 = b'c';

    /// Test signal: connected to DC.
    pub const TEST_SIGNAL_DC: u8 = b'-';

    /// Test signal: connected to internal 1x amplitude.
    pub const TEST_SIGNAL_1X: u8 = b'=';

    /// Test signal: connected to internal 2x amplitude.
    pub const TEST_SIGNAL_2X: u8 = b'p';

    /// Channel gain settings (1-8).
    pub const CHANNEL_SETTINGS: [u8; 8] = [b'1', b'2', b'3', b'4', b'5', b'6', b'7', b'8'];

    /// Daisy channel settings (9-16).
    pub const DAISY_CHANNEL_SETTINGS: [u8; 8] = [b'Q', b'W', b'E', b'R', b'T', b'Y', b'U', b'I'];

    /// Channel power-off commands.
    pub const CHANNEL_OFF: [u8; 8] = [b'1', b'2', b'3', b'4', b'5', b'6', b'7', b'8'];

    /// Channel power-on commands.
    pub const CHANNEL_ON: [u8; 8] = [b'!', b'@', b'#', b'$', b'%', b'^', b'&', b'*'];

    /// Daisy channel power-off commands.
    pub const DAISY_OFF: [u8; 8] = [b'q', b'w', b'e', b'r', b't', b'y', b'u', b'i'];

    /// Daisy channel power-on commands.
    pub const DAISY_ON: [u8; 8] = [b'Q', b'W', b'E', b'R', b'T', b'Y', b'U', b'I'];
}

#[cfg(test)]
mod tests {
    use super::*;
    use alloc::vec;

    #[test]
    fn test_parse_int24_positive() {
        // Positive value: 0x000001 = 1
        let data = [0x00, 0x00, 0x01];
        assert_eq!(parse_int24(&data), 1);

        // Larger positive: 0x7FFFFF = 8388607
        let data = [0x7F, 0xFF, 0xFF];
        assert_eq!(parse_int24(&data), 8388607);
    }

    #[test]
    fn test_parse_int24_negative() {
        // Negative value: 0xFFFFFF = -1
        let data = [0xFF, 0xFF, 0xFF];
        assert_eq!(parse_int24(&data), -1);

        // 0x800000 = -8388608
        let data = [0x80, 0x00, 0x00];
        assert_eq!(parse_int24(&data), -8388608);
    }

    #[test]
    fn test_parse_int16() {
        let data = [0x00, 0x01];
        assert_eq!(parse_int16(&data), 1);

        let data = [0xFF, 0xFF];
        assert_eq!(parse_int16(&data), -1);
    }

    #[test]
    fn test_parse_cyton_packet() {
        // Create a valid test packet
        let mut packet = vec![0u8; CYTON_PACKET_SIZE];
        packet[0] = HEADER_BYTE;
        packet[1] = 42; // Sample number
        packet[CYTON_PACKET_SIZE - 1] = FOOTER_STANDARD;

        // Set channel 1 to a known value
        packet[2] = 0x00;
        packet[3] = 0x00;
        packet[4] = 0x01;

        let parsed = OpenBciPacket::parse_cyton(&packet, 24.0).unwrap();
        assert_eq!(parsed.sample_number, 42);
        assert_eq!(parsed.eeg_channels.len(), 8);
        assert_eq!(parsed.aux_channels.len(), 3);
        assert_eq!(parsed.packet_type, PacketType::Standard);
    }

    #[test]
    fn test_invalid_header() {
        let mut packet = vec![0u8; CYTON_PACKET_SIZE];
        packet[0] = 0x00; // Invalid header

        assert!(OpenBciPacket::parse_cyton(&packet, 24.0).is_none());
    }
}
