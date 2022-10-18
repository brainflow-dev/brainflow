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
    ip_address_aux: String,
    ip_address_anc: String,
    ip_port: usize,
    ip_port_aux: usize,
    ip_port_anc: usize,
    ip_protocol: usize,
    other_info: String,
    timeout: usize,
    serial_number: String,
    file: String,
    file_aux: String,
    file_anc: String,
    master_board: usize,
}

impl Default for BrainFlowInputParams {
    fn default() -> Self {
        Self {
            serial_port: "".to_string(),
            mac_address: "".to_string(),
            ip_address: "".to_string(),
            ip_address_aux: "".to_string(),
            ip_address_anc: "".to_string(),
            ip_port: 0,
            ip_port_anc: 0,
            ip_port_aux: 0,
            ip_protocol: 0,
            other_info: "".to_string(),
            timeout: 0,
            serial_number: "".to_string(),
            file: "".to_string(),
            file_aux: "".to_string(),
            file_anc: "".to_string(),
            master_board: BoardIds::NoBoard as usize,
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

    /// IP address is used for boards which reads data from socket connection.
    pub fn ip_address_aux<S: AsRef<str>>(mut self, address: S) -> Self {
        self.params.ip_address_aux = address.as_ref().to_string();
        self
    }

    /// IP address is used for boards which reads data from socket connection.
    pub fn ip_address_anc<S: AsRef<str>>(mut self, address: S) -> Self {
        self.params.ip_address_anc = address.as_ref().to_string();
        self
    }

    /// IP port for socket connection, for some boards where we know it in front you dont need this parameter.
    pub fn ip_port(mut self, port: usize) -> Self {
        self.params.ip_port = port;
        self
    }

    /// IP port for socket connection, for some boards where we know it in front you dont need this parameter.
    pub fn ip_port_aux(mut self, port: usize) -> Self {
        self.params.ip_port_aux = port;
        self
    }

    /// IP port for socket connection, for some boards where we know it in front you dont need this parameter.
    pub fn ip_port_anc(mut self, port: usize) -> Self {
        self.params.ip_port_anc = port;
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

    pub fn file_aux<S: AsRef<str>>(mut self, filename: S) -> Self {
        self.params.file_aux = filename.as_ref().to_string();
        self
    }

    pub fn file_anc<S: AsRef<str>>(mut self, filename: S) -> Self {
        self.params.file_anc = filename.as_ref().to_string();
        self
    }

    /// Build BrainFlowInputParams with the given options.
    pub fn build(self) -> BrainFlowInputParams {
        self.params
    }
}
