use serde::{Deserialize, Serialize};

#[derive(Debug, Default, PartialEq, Eq, PartialOrd, Ord, Serialize, Deserialize)]
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
