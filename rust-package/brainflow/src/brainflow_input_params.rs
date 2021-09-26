use getset::Getters;
use serde::{Deserialize, Serialize};

#[derive(Debug, Default, PartialEq, Eq, PartialOrd, Ord, Serialize, Deserialize, Getters)]
#[getset(get = "pub", set = "pub")]
pub struct BrainFlowInputParams {
    serial_port: String,
    mac_address: String,
    ip_address: String,
    ip_port: usize,
    ip_protocol: i32,
    other_info: String,
    timeout: usize,
    serial_number: String,
    file: String,
}

#[derive(Default)]
pub struct BrainFlowInputParamsBuilder {
    params: BrainFlowInputParams,
}

impl BrainFlowInputParamsBuilder {
    pub fn new() -> Self {
        Default::default()
    }

    pub fn serial_port<S: AsRef<str>>(mut self, port: S) -> Self {
        self.params.serial_port = port.as_ref().to_string();
        self
    }

    pub fn mac_address<S: AsRef<str>>(mut self, address: S) -> Self {
        self.params.mac_address = address.as_ref().to_string();
        self
    }

    pub fn ip_address<S: AsRef<str>>(mut self, address: S) -> Self {
        self.params.ip_address = address.as_ref().to_string();
        self
    }

    pub fn ip_port(mut self, port: usize) -> Self {
        self.params.ip_port = port;
        self
    }

    pub fn ip_protocol(mut self, protocol: i32) -> Self {
        self.params.ip_protocol = protocol;
        self
    }

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

    pub fn build(self) -> BrainFlowInputParams {
        self.params
    }
}
