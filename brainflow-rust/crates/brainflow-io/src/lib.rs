//! Pure Rust I/O abstractions for BrainFlow.
//!
//! This crate provides platform-specific implementations for:
//! - Serial port communication
//! - TCP/UDP sockets
//! - Bluetooth/BLE communication

#![no_std]

extern crate alloc;

#[cfg(feature = "std")]
extern crate std;

pub mod buffer;

// TODO: Implement serial and socket modules for device communication
// #[cfg(feature = "std")]
// pub mod serial;
// #[cfg(feature = "std")]
// pub mod socket;

pub use buffer::RingBuffer;

/// I/O error types.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum IoError {
    /// Connection failed.
    ConnectionFailed,
    /// Read timeout.
    Timeout,
    /// Write failed.
    WriteFailed,
    /// Read failed.
    ReadFailed,
    /// Port not open.
    NotOpen,
    /// Port already open.
    AlreadyOpen,
    /// Invalid configuration.
    InvalidConfig,
    /// Device not found.
    DeviceNotFound,
    /// Permission denied.
    PermissionDenied,
    /// Other error.
    Other,
}

/// Result type for I/O operations.
pub type IoResult<T> = Result<T, IoError>;

/// Trait for byte stream sources.
pub trait ByteReader {
    /// Read bytes into a buffer.
    fn read(&mut self, buf: &mut [u8]) -> IoResult<usize>;

    /// Read with timeout in milliseconds.
    fn read_timeout(&mut self, buf: &mut [u8], timeout_ms: u32) -> IoResult<usize>;

    /// Check how many bytes are available.
    fn bytes_available(&self) -> IoResult<usize>;
}

/// Trait for byte stream sinks.
pub trait ByteWriter {
    /// Write bytes from a buffer.
    fn write(&mut self, buf: &[u8]) -> IoResult<usize>;

    /// Flush any buffered output.
    fn flush(&mut self) -> IoResult<()>;
}
