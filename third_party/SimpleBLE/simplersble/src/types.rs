use std::fmt;

#[derive(Debug, Clone)]
pub struct Error {
    msg: String,
}

impl Error {
    // Helper function to create an error from a C++ exception string
    #[allow(dead_code)]
    pub(crate) fn from_string(s: String) -> Self {
        Error { msg: s }
    }

    // Helper function to create an error from a C++ exception
    pub(crate) fn from_cxx_exception(e: cxx::Exception) -> Self {
        Error { msg: e.what().to_string() }
    }
}

#[derive(Debug)]
pub enum BluetoothAddressType {
    Public,
    Random,
    Unspecified,
}

#[derive(Debug)]
pub enum CharacteristicCapability {
    Read,
    WriteRequest,
    WriteCommand,
    Notify,
    Indicate,
}

impl std::error::Error for Error {}

impl fmt::Display for BluetoothAddressType {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            BluetoothAddressType::Public => write!(f, "Public"),
            BluetoothAddressType::Random => write!(f, "Random"),
            BluetoothAddressType::Unspecified => write!(f, "Unspecified"),
        }
    }
}

impl fmt::Display for CharacteristicCapability {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            CharacteristicCapability::Read => write!(f, "Read"),
            CharacteristicCapability::WriteRequest => write!(f, "WriteRequest"),
            CharacteristicCapability::WriteCommand => write!(f, "WriteCommand"),
            CharacteristicCapability::Notify => write!(f, "Notify"),
            CharacteristicCapability::Indicate => write!(f, "Indicate"),
        }
    }
}

impl fmt::Display for Error {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "SimpleBLE error: {}", self.msg)
    }
}