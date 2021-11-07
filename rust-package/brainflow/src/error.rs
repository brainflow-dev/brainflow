use std::str::Utf8Error;

use thiserror::Error;

#[derive(Debug, Error)]
pub enum Error {
    #[error("{0}")]
    BrainFlowError(#[from] BrainFlowError),

    #[error("Interior nul byte found")]
    NulError(#[from] std::ffi::NulError),

    /// Some JSON error
    #[error("JSON error")]
    JsonError(#[from] serde_json::Error),

    #[error("Cannot convert from Utf8")]
    Utf8Error(#[from] Utf8Error),

    #[error("{0}")]
    ShapeError(#[from] ndarray::ShapeError),

    #[error("{0}")]
    FromPrimitive(#[from] std::num::ParseIntError),
}

#[derive(Debug, Error)]
pub enum BrainFlowError {
    #[error("")]
    PortAlreadyOpenError,

    #[error("")]
    UnableToOpenPortError,

    #[error("")]
    SetPortError,

    #[error("")]
    BoardWriteError,

    #[error("")]
    IncommingMsgError,

    #[error("")]
    InitialMsgError,

    #[error("")]
    BoardNotReadyError,

    #[error("")]
    StreamAlreadyRunError,

    #[error("")]
    InvalidBufferSizeError,

    #[error("")]
    StreamThreadError,

    #[error("")]
    StreamThreadIsNotRunning,

    #[error("")]
    EmptyBufferError,

    #[error("")]
    InvalidArgumentsError,

    #[error("")]
    UnsupportedBoardError,

    #[error("")]
    BoardNotCreatedError,

    #[error("")]
    AnotherBoardIsCreatedError,

    #[error("")]
    GeneralError,

    #[error("")]
    SyncTimeoutError,

    #[error("")]
    JsonNotFoundError,

    #[error("")]
    NoSuchDataInJsonError,

    #[error("")]
    ClassifierIsNotPreparedError,

    #[error("")]
    AnotherClassifierIsPreparedError,

    #[error("")]
    UnsupportedClassifierAndMetricCombinationError,

    #[error("This error code is not (yet) supported by this Rust binding")]
    ErrorIsNotSupportedInRustError,
}
