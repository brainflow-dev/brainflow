//! Board implementations for BrainFlow.
//!
//! This crate provides:
//! - Board trait and common types
//! - Synthetic board for testing
//! - Board controller for managing board instances

#![no_std]

extern crate alloc;

#[cfg(feature = "std")]
extern crate std;

use alloc::string::String;
use alloc::vec::Vec;
use brainflow_sys::error::{Error, ErrorCode, Result};

pub mod board;
pub mod controller;
pub mod openbci;
pub mod preset;
pub mod synthetic;

pub use board::{Board, BoardDescription, BoardState};
pub use controller::BoardController;
pub use preset::Preset;
pub use synthetic::SyntheticBoard;

/// Board IDs matching the C++ implementation.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
#[repr(i32)]
pub enum BoardId {
    /// Playback file board.
    PlaybackFile = -3,
    /// Streaming board (receives data over network).
    Streaming = -2,
    /// Synthetic board (generates test data).
    Synthetic = -1,
    /// OpenBCI Cyton board.
    Cyton = 0,
    /// OpenBCI Ganglion board.
    Ganglion = 1,
    /// OpenBCI Cyton + Daisy.
    CytonDaisy = 2,
    /// Galea board.
    Galea = 3,
    /// OpenBCI Ganglion WiFi.
    GanglionWifi = 4,
    /// OpenBCI Cyton WiFi.
    CytonWifi = 5,
    /// OpenBCI Cyton + Daisy WiFi.
    CytonDaisyWifi = 6,
    /// BrainBit board.
    BrainBit = 7,
    /// Unicorn board.
    Unicorn = 8,
    /// Callibri EEG.
    CallibriEeg = 9,
    /// Callibri EMG.
    CallibriEmg = 10,
    /// Callibri ECG.
    CallibriEcg = 11,
    /// Fascia board.
    Fascia = 12,
    /// Notion 1.
    Notion1 = 13,
    /// Notion 2.
    Notion2 = 14,
    /// Ironbci board.
    Ironbci = 15,
    /// gForce Pro.
    GforcePro = 16,
    /// FreeEEG32 board.
    FreeEeg32 = 17,
    /// BrainBit BLED.
    BrainBitBled = 18,
    /// gForce Dual.
    GforceDual = 19,
    /// Galea Serial.
    GaleaSerial = 20,
    /// Muse S BLED.
    MuseSBled = 21,
    /// Muse 2 BLED.
    Muse2Bled = 22,
    /// Crown board.
    Crown = 23,
    /// ANT Neuro EE 410.
    AntNeuroEe410 = 24,
    /// ANT Neuro EE 411.
    AntNeuroEe411 = 25,
    /// ANT Neuro EE 430.
    AntNeuroEe430 = 26,
    /// ANT Neuro EE 211.
    AntNeuroEe211 = 27,
    /// ANT Neuro EE 212.
    AntNeuroEe212 = 28,
    /// ANT Neuro EE 213.
    AntNeuroEe213 = 29,
    /// ANT Neuro EE 214.
    AntNeuroEe214 = 30,
    /// ANT Neuro EE 215.
    AntNeuroEe215 = 31,
    /// ANT Neuro EE 221.
    AntNeuroEe221 = 32,
    /// ANT Neuro EE 222.
    AntNeuroEe222 = 33,
    /// ANT Neuro EE 223.
    AntNeuroEe223 = 34,
    /// ANT Neuro EE 224.
    AntNeuroEe224 = 35,
    /// ANT Neuro EE 225.
    AntNeuroEe225 = 36,
    /// Enophone board.
    Enophone = 37,
    /// Muse 2 board.
    Muse2 = 38,
    /// Muse S board.
    MuseS = 39,
    /// BrainAlive board.
    BrainAlive = 40,
    /// Muse 2016 board.
    Muse2016 = 41,
    /// Muse 2016 BLED.
    Muse2016Bled = 42,
    /// PIEEG board.
    Pieeg = 43,
    /// Explore 4 channels.
    Explore4Chan = 44,
    /// Explore 8 channels.
    Explore8Chan = 45,
    /// Ganglion native.
    GanglionNative = 46,
    /// EmotiBit.
    Emotibit = 47,
    /// Galea V4.
    GaleaV4 = 48,
    /// Galea V4 Serial.
    GaleaV4Serial = 49,
    /// NTL WiFi.
    NtlWifi = 50,
    /// ANT Neuro EE 511.
    AntNeuroEe511 = 51,
    /// FreeEEG 128.
    FreeEeg128 = 52,
    /// AAVAA V3.
    AavaaV3 = 53,
    /// Explore 32 channels.
    Explore32Chan = 54,
    /// Explore Plus 8 channels.
    ExplorePlus8Chan = 55,
    /// Explore Plus 32 channels.
    ExplorePlus32Chan = 56,
    /// Neuropawn Knight.
    NeuropawnKnight = 57,
    /// Synchroni boards (58-63).
    Synchroni = 58,
}

impl BoardId {
    /// Convert from raw i32 value.
    pub fn from_raw(value: i32) -> Option<Self> {
        // This is a simplified version - in practice you'd want a complete match
        match value {
            -3 => Some(Self::PlaybackFile),
            -2 => Some(Self::Streaming),
            -1 => Some(Self::Synthetic),
            0 => Some(Self::Cyton),
            1 => Some(Self::Ganglion),
            2 => Some(Self::CytonDaisy),
            // ... add remaining mappings
            _ => None,
        }
    }

    /// Convert to raw i32 value.
    pub fn to_raw(self) -> i32 {
        self as i32
    }
}

/// Input parameters for board initialization.
#[derive(Debug, Clone, Default)]
pub struct BoardParams {
    /// Serial port name (e.g., "/dev/ttyUSB0", "COM3").
    pub serial_port: Option<String>,
    /// MAC address for Bluetooth devices.
    pub mac_address: Option<String>,
    /// IP address for network devices.
    pub ip_address: Option<String>,
    /// IP port for network devices.
    pub ip_port: Option<u16>,
    /// Connection timeout in seconds.
    pub timeout: Option<u32>,
    /// File path for playback boards.
    pub file: Option<String>,
    /// Serial number for device identification.
    pub serial_number: Option<String>,
    /// Master board ID for streaming boards.
    pub master_board: Option<BoardId>,
}

impl BoardParams {
    /// Create new empty parameters.
    pub fn new() -> Self {
        Self::default()
    }

    /// Set serial port.
    pub fn with_serial_port(mut self, port: impl Into<String>) -> Self {
        self.serial_port = Some(port.into());
        self
    }

    /// Set MAC address.
    pub fn with_mac_address(mut self, mac: impl Into<String>) -> Self {
        self.mac_address = Some(mac.into());
        self
    }

    /// Set IP address.
    pub fn with_ip_address(mut self, ip: impl Into<String>) -> Self {
        self.ip_address = Some(ip.into());
        self
    }

    /// Set IP port.
    pub fn with_ip_port(mut self, port: u16) -> Self {
        self.ip_port = Some(port);
        self
    }

    /// Set timeout.
    pub fn with_timeout(mut self, timeout_secs: u32) -> Self {
        self.timeout = Some(timeout_secs);
        self
    }

    /// Set file path.
    pub fn with_file(mut self, file: impl Into<String>) -> Self {
        self.file = Some(file.into());
        self
    }
}
