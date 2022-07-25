use crate::BrainFlowPresets;
use crate::BoardIds;
use getset::Getters;
use serde::{Deserialize, Serialize};

use crate::IpProtocolTypes;

/// Input parameters for [crate::board_shim::BoardShim].
#[derive(Debug, PartialEq, Eq, PartialOrd, Ord, Serialize, Deserialize, Getters)]
#[getset(get = "pub", set = "pub")]
pub struct BrainFlowInputParams {
    serial_port: String,
    mac_address: String,
    ip_address: String,
    ip_port: usize,
    ip_protocol: usize,
    other_info: String,
    timeout: usize,
    serial_number: String,
    file: String,
    master_board: usize,
    preset: usize,
}

impl Default for BrainFlowInputParams {
    fn default() -> Self {
        Self {
            serial_port: "".to_string(),
            mac_address: "".to_string(),
            ip_address: "".to_string(),
            ip_port: 0,
            ip_protocol: 0,
            other_info: "".to_string(),
            timeout: 0,
            serial_number: "".to_string(),
            file: "".to_string(),
            master_board: BoardIds::NoBoard as usize,
            preset: BrainFlowPresets::DefaultPreset as usize,
        }
    }
}

/// Builder for [BrainFlowInputParams].
#[derive(Default)]
pub struct BrainFlowInputParamsBuilder {
    params: BrainFlowInputParams,
}

impl BrainFlowInputParamsBuilder {
    /// Create a new builder.
    pub fn new() -> Self {
        Default::default()
    }
    /// Serial port name is used for boards which reads data from serial port.
    pub fn serial_port<S: AsRef<str>>(mut self, port: S) -> Self {
        self.params.serial_port = port.as_ref().to_string();
        self
    }

    /// Mac address, for example, is used for bluetooth based boards.
    pub fn mac_address<S: AsRef<str>>(mut self, address: S) -> Self {
        self.params.mac_address = address.as_ref().to_string();
        self
    }

    /// IP address is used for boards which reads data from socket connection.
    pub fn ip_address<S: AsRef<str>>(mut self, address: S) -> Self {
        self.params.ip_address = address.as_ref().to_string();
        self
    }

    /// IP port for socket connection, for some boards where we know it in front you dont need this parameter.
    pub fn ip_port(mut self, port: usize) -> Self {
        self.params.ip_port = port;
        self
    }

    /// IP protocol type from IpProtocolType enum.
    pub fn ip_protocol(mut self, protocol: IpProtocolTypes) -> Self {
        self.params.ip_protocol = protocol as usize;
        self
    }

    /// Master Board from BoardIds enum.
    pub fn master_board(mut self, master_board: BoardIds) -> Self {
        self.params.master_board = master_board as usize;
        self
    }

    /// Preset type from BrainFlowPresets enum.
    pub fn preset(mut self, preset: BrainFlowPresets) -> Self {
        self.params.preset = preset as usize;
        self
    }

    /// Other info.
    pub fn other_info<S: AsRef<str>>(mut self, info: S) -> Self {
        self.params.other_info = info.as_ref().to_string();
        self
    }

    pub fn timeout(mut self, timeout: usize) -> Self {
        self.params.timeout = timeout;
        self
    }

    pub fn serial_number<S: AsRef<str>>(mut self, serial: S) -> Self {
        self.params.serial_number = serial.as_ref().to_string();
        self
    }

    pub fn file<S: AsRef<str>>(mut self, filename: S) -> Self {
        self.params.file = filename.as_ref().to_string();
        self
    }

    /// Build BrainFlowInputParams with the given options.
    pub fn build(self) -> BrainFlowInputParams {
        self.params
    }
}
