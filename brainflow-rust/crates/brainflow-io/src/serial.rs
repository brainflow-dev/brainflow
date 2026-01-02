//! Serial port communication abstraction.
//!
//! Provides cross-platform serial port communication for biosensor devices.

use alloc::string::String;
use alloc::vec::Vec;

use crate::IoError;

/// Serial port configuration.
#[derive(Debug, Clone)]
pub struct SerialConfig {
    /// Baud rate (e.g., 115200, 19200)
    pub baud_rate: u32,
    /// Read timeout in milliseconds
    pub timeout_ms: u32,
    /// Data bits (typically 8)
    pub data_bits: u8,
    /// Stop bits (1 or 2)
    pub stop_bits: u8,
    /// Parity: 0=none, 1=odd, 2=even
    pub parity: u8,
    /// Hardware flow control
    pub flow_control: bool,
}

impl Default for SerialConfig {
    fn default() -> Self {
        Self {
            baud_rate: 115200,
            timeout_ms: 1000,
            data_bits: 8,
            stop_bits: 1,
            parity: 0,
            flow_control: false,
        }
    }
}

impl SerialConfig {
    /// Create config for OpenBCI Cyton boards.
    pub fn openbci_cyton() -> Self {
        Self {
            baud_rate: 115200,
            timeout_ms: 1000,
            ..Default::default()
        }
    }

    /// Create config for FreeEEG boards.
    pub fn freeeeg() -> Self {
        Self {
            baud_rate: 921600,
            timeout_ms: 1000,
            ..Default::default()
        }
    }
}

/// Serial port trait for cross-platform abstraction.
pub trait SerialPort: Send + Sync {
    /// Open the serial port with the given configuration.
    fn open(&mut self, config: &SerialConfig) -> Result<(), IoError>;

    /// Check if the port is open.
    fn is_open(&self) -> bool;

    /// Read bytes from the serial port.
    /// Returns the number of bytes read.
    fn read(&mut self, buf: &mut [u8]) -> Result<usize, IoError>;

    /// Write bytes to the serial port.
    /// Returns the number of bytes written.
    fn write(&mut self, data: &[u8]) -> Result<usize, IoError>;

    /// Flush the input/output buffers.
    fn flush(&mut self) -> Result<(), IoError>;

    /// Close the serial port.
    fn close(&mut self) -> Result<(), IoError>;

    /// Get the port name.
    fn port_name(&self) -> &str;

    /// Set read timeout in milliseconds.
    fn set_timeout(&mut self, timeout_ms: u32) -> Result<(), IoError>;

    /// Get number of bytes available to read.
    fn bytes_available(&self) -> Result<usize, IoError>;
}

/// Platform-specific serial port implementation.
#[cfg(all(feature = "std", target_family = "unix"))]
pub mod unix {
    use super::*;
    use std::fs::{File, OpenOptions};
    use std::io::{Read, Write as IoWrite};
    use std::os::unix::io::{AsRawFd, RawFd};

    /// Unix serial port using termios.
    pub struct UnixSerialPort {
        port_name: String,
        file: Option<File>,
        config: SerialConfig,
    }

    impl UnixSerialPort {
        /// Create a new Unix serial port.
        pub fn new(port_name: &str) -> Self {
            Self {
                port_name: String::from(port_name),
                file: None,
                config: SerialConfig::default(),
            }
        }

        #[cfg(target_os = "linux")]
        fn configure_termios(&self, fd: RawFd, config: &SerialConfig) -> Result<(), IoError> {
            use std::mem::MaybeUninit;

            // This would use libc termios in a real implementation
            // For now, return Ok as placeholder
            Ok(())
        }
    }

    impl SerialPort for UnixSerialPort {
        fn open(&mut self, config: &SerialConfig) -> Result<(), IoError> {
            let file = OpenOptions::new()
                .read(true)
                .write(true)
                .open(&self.port_name)
                .map_err(|_| IoError::OpenFailed)?;

            #[cfg(target_os = "linux")]
            self.configure_termios(file.as_raw_fd(), config)?;

            self.file = Some(file);
            self.config = config.clone();
            Ok(())
        }

        fn is_open(&self) -> bool {
            self.file.is_some()
        }

        fn read(&mut self, buf: &mut [u8]) -> Result<usize, IoError> {
            let file = self.file.as_mut().ok_or(IoError::NotConnected)?;
            file.read(buf).map_err(|_| IoError::ReadFailed)
        }

        fn write(&mut self, data: &[u8]) -> Result<usize, IoError> {
            let file = self.file.as_mut().ok_or(IoError::NotConnected)?;
            file.write(data).map_err(|_| IoError::WriteFailed)
        }

        fn flush(&mut self) -> Result<(), IoError> {
            let file = self.file.as_mut().ok_or(IoError::NotConnected)?;
            file.flush().map_err(|_| IoError::WriteFailed)
        }

        fn close(&mut self) -> Result<(), IoError> {
            self.file = None;
            Ok(())
        }

        fn port_name(&self) -> &str {
            &self.port_name
        }

        fn set_timeout(&mut self, timeout_ms: u32) -> Result<(), IoError> {
            self.config.timeout_ms = timeout_ms;
            Ok(())
        }

        fn bytes_available(&self) -> Result<usize, IoError> {
            // Would use FIONREAD ioctl in real implementation
            Ok(0)
        }
    }
}

/// No-std stub implementation for embedded systems.
#[cfg(not(feature = "std"))]
pub mod nostd {
    use super::*;

    /// Stub serial port for no_std environments.
    /// Real implementations would be provided by the HAL.
    pub struct StubSerialPort {
        port_name: String,
        is_open: bool,
    }

    impl StubSerialPort {
        /// Create a new stub serial port.
        pub fn new(port_name: &str) -> Self {
            Self {
                port_name: String::from(port_name),
                is_open: false,
            }
        }
    }

    impl SerialPort for StubSerialPort {
        fn open(&mut self, _config: &SerialConfig) -> Result<(), IoError> {
            self.is_open = true;
            Ok(())
        }

        fn is_open(&self) -> bool {
            self.is_open
        }

        fn read(&mut self, _buf: &mut [u8]) -> Result<usize, IoError> {
            if !self.is_open {
                return Err(IoError::NotConnected);
            }
            Ok(0)
        }

        fn write(&mut self, data: &[u8]) -> Result<usize, IoError> {
            if !self.is_open {
                return Err(IoError::NotConnected);
            }
            Ok(data.len())
        }

        fn flush(&mut self) -> Result<(), IoError> {
            Ok(())
        }

        fn close(&mut self) -> Result<(), IoError> {
            self.is_open = false;
            Ok(())
        }

        fn port_name(&self) -> &str {
            &self.port_name
        }

        fn set_timeout(&mut self, _timeout_ms: u32) -> Result<(), IoError> {
            Ok(())
        }

        fn bytes_available(&self) -> Result<usize, IoError> {
            Ok(0)
        }
    }
}

/// Enumerate available serial ports.
#[cfg(feature = "std")]
pub fn list_ports() -> Vec<String> {
    use alloc::string::ToString;
    use std::format;

    // Platform-specific port enumeration
    #[cfg(target_os = "linux")]
    {
        use std::fs;
        let mut ports = Vec::new();
        if let Ok(entries) = fs::read_dir("/dev") {
            for entry in entries.flatten() {
                let name = entry.file_name().to_string_lossy().into_owned();
                if name.starts_with("ttyUSB") || name.starts_with("ttyACM") {
                    ports.push(format!("/dev/{}", name));
                }
            }
        }
        ports
    }

    #[cfg(target_os = "macos")]
    {
        use std::fs;
        let mut ports = Vec::new();
        if let Ok(entries) = fs::read_dir("/dev") {
            for entry in entries.flatten() {
                let name = entry.file_name().to_string_lossy().into_owned();
                if name.starts_with("cu.usbserial") || name.starts_with("cu.usbmodem") {
                    ports.push(format!("/dev/{}", name));
                }
            }
        }
        ports
    }

    #[cfg(target_os = "windows")]
    {
        // Would enumerate COM ports via registry
        Vec::new()
    }

    #[cfg(not(any(target_os = "linux", target_os = "macos", target_os = "windows")))]
    {
        Vec::new()
    }
}

#[cfg(not(feature = "std"))]
pub fn list_ports() -> Vec<String> {
    Vec::new()
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_serial_config_default() {
        let config = SerialConfig::default();
        assert_eq!(config.baud_rate, 115200);
        assert_eq!(config.data_bits, 8);
        assert_eq!(config.stop_bits, 1);
    }

    #[test]
    fn test_openbci_config() {
        let config = SerialConfig::openbci_cyton();
        assert_eq!(config.baud_rate, 115200);
    }
}
