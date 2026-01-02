//! Error types for BrainFlow.

use alloc::string::String;
use core::fmt;

/// BrainFlow error codes matching the C++ implementation.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
#[repr(i32)]
pub enum ErrorCode {
    /// Operation completed successfully.
    Ok = 0,
    /// Port is already open.
    PortAlreadyOpen = 1,
    /// Unable to open port.
    UnableToOpenPort = 2,
    /// Set port settings failed.
    SetPortSettingsFailed = 3,
    /// Board write failed.
    BoardWriteFailed = 4,
    /// Incomming message corrupted.
    IncommingMsgCorrupted = 5,
    /// Initial message corrupted.
    InitialMsgCorrupted = 6,
    /// Board not ready.
    BoardNotReady = 7,
    /// Stream already running.
    StreamAlreadyRunning = 8,
    /// Invalid buffer size.
    InvalidBufferSize = 9,
    /// Stream thread is not running.
    StreamThreadNotRunning = 10,
    /// Stream thread exited.
    StreamThreadExited = 11,
    /// Invalid arguments.
    InvalidArguments = 12,
    /// Unsupported board.
    UnsupportedBoard = 13,
    /// Board not created.
    BoardNotCreated = 14,
    /// Another board is created.
    AnotherBoardCreated = 15,
    /// General error.
    GeneralError = 16,
    /// Sync timeout.
    SyncTimeout = 17,
    /// Json not found.
    JsonNotFound = 18,
    /// No data in buffer.
    NoDataInBuffer = 19,
    /// Classifier not prepared.
    ClassifierNotPrepared = 20,
    /// Another classifier prepared.
    AnotherClassifierPrepared = 21,
    /// Unsupported classifier.
    UnsupportedClassifier = 22,
    /// Failed to send data.
    FailedToSendData = 23,
    /// Failed to receive data.
    FailedToReceiveData = 24,
}

impl ErrorCode {
    /// Convert from raw i32 value.
    #[must_use]
    pub const fn from_raw(code: i32) -> Option<Self> {
        match code {
            0 => Some(Self::Ok),
            1 => Some(Self::PortAlreadyOpen),
            2 => Some(Self::UnableToOpenPort),
            3 => Some(Self::SetPortSettingsFailed),
            4 => Some(Self::BoardWriteFailed),
            5 => Some(Self::IncommingMsgCorrupted),
            6 => Some(Self::InitialMsgCorrupted),
            7 => Some(Self::BoardNotReady),
            8 => Some(Self::StreamAlreadyRunning),
            9 => Some(Self::InvalidBufferSize),
            10 => Some(Self::StreamThreadNotRunning),
            11 => Some(Self::StreamThreadExited),
            12 => Some(Self::InvalidArguments),
            13 => Some(Self::UnsupportedBoard),
            14 => Some(Self::BoardNotCreated),
            15 => Some(Self::AnotherBoardCreated),
            16 => Some(Self::GeneralError),
            17 => Some(Self::SyncTimeout),
            18 => Some(Self::JsonNotFound),
            19 => Some(Self::NoDataInBuffer),
            20 => Some(Self::ClassifierNotPrepared),
            21 => Some(Self::AnotherClassifierPrepared),
            22 => Some(Self::UnsupportedClassifier),
            23 => Some(Self::FailedToSendData),
            24 => Some(Self::FailedToReceiveData),
            _ => None,
        }
    }

    /// Convert to raw i32 value.
    #[must_use]
    pub const fn to_raw(self) -> i32 {
        self as i32
    }
}

/// BrainFlow error type.
#[derive(Debug, Clone)]
pub struct Error {
    /// The error code.
    pub code: ErrorCode,
    /// Optional error message.
    pub message: Option<String>,
}

impl Error {
    /// Create a new error with just a code.
    #[must_use]
    pub const fn new(code: ErrorCode) -> Self {
        Self {
            code,
            message: None,
        }
    }

    /// Create a new error with a code and message.
    #[must_use]
    pub fn with_message(code: ErrorCode, message: impl Into<String>) -> Self {
        Self {
            code,
            message: Some(message.into()),
        }
    }
}

impl fmt::Display for Error {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "BrainFlow error {:?}", self.code)?;
        if let Some(ref msg) = self.message {
            write!(f, ": {msg}")?;
        }
        Ok(())
    }
}

/// Result type alias for BrainFlow operations.
pub type Result<T> = core::result::Result<T, Error>;
