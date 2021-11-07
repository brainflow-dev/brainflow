#[cfg(test)]
#[macro_use]
extern crate approx;

extern crate num;
extern crate num_derive;

use error::Error;

/// The primary interface to all boards.
pub mod board_shim;
/// Input parameters for [board_shim::BoardShim].
pub mod brainflow_input_params;
pub mod error;

/// Input parameters for [ml_model::MLModel].
pub mod brainflow_model_params;
/// Methods for signal processig.
pub mod data_filter;
mod ffi;
/// Used to calculate derivative metrics from raw data.
pub mod ml_model;

/// Store all supported BrainFlow Errors.
pub use error::BrainFlowError;
/// Enum to store all supported Agg Operations.
pub use ffi::constants::AggOperations;
/// Enum to store all supported Board Ids.
pub use ffi::constants::BoardIds;
/// Enum to store all supported BrainFlow Classifiers.
pub use ffi::constants::BrainFlowClassifiers;
/// Enum to store all Brainflow Exit Codes.
pub use ffi::constants::BrainFlowExitCodes;
/// Enum to store BrainFlow metrics
pub use ffi::constants::BrainFlowMetrics;
/// Enum to store all supported Detrend Operations.
pub use ffi::constants::DetrendOperations;
/// Enum to store all supported Filter Types.
pub use ffi::constants::FilterTypes;
/// Enum to store all supported Ip Protocol Types.
pub use ffi::constants::IpProtocolType;
/// Enum to store all supported Log Levels.
pub use ffi::constants::LogLevels;
/// Enum to store all supported Noise Types.
pub use ffi::constants::NoiseTypes;
/// Enum to store all supported Window Functions.
pub use ffi::constants::WindowFunctions;

type BrainFlowExitCode = i32;

/// Convert the brainflow exit code to [BrainFlowError].
fn check_brainflow_exit_code(value: BrainFlowExitCode) -> std::result::Result<(), BrainFlowError> {
    if value == 0 {
        Ok(())
    } else {
        Err(match value {
            1 => BrainFlowError::PortAlreadyOpenError,
            2 => BrainFlowError::UnableToOpenPortError,
            3 => BrainFlowError::SetPortError,
            4 => BrainFlowError::BoardWriteError,
            5 => BrainFlowError::IncommingMsgError,
            6 => BrainFlowError::InitialMsgError,
            7 => BrainFlowError::BoardNotReadyError,
            8 => BrainFlowError::StreamAlreadyRunError,
            9 => BrainFlowError::InvalidBufferSizeError,
            10 => BrainFlowError::StreamThreadError,
            11 => BrainFlowError::StreamThreadIsNotRunning,
            12 => BrainFlowError::EmptyBufferError,
            13 => BrainFlowError::InvalidArgumentsError,
            14 => BrainFlowError::UnsupportedBoardError,
            15 => BrainFlowError::BoardNotCreatedError,
            16 => BrainFlowError::AnotherBoardIsCreatedError,
            17 => BrainFlowError::GeneralError,
            18 => BrainFlowError::SyncTimeoutError,
            19 => BrainFlowError::JsonNotFoundError,
            20 => BrainFlowError::NoSuchDataInJsonError,
            21 => BrainFlowError::ClassifierIsNotPreparedError,
            22 => BrainFlowError::AnotherClassifierIsPreparedError,
            23 => BrainFlowError::UnsupportedClassifierAndMetricCombinationError,
            _ => BrainFlowError::ErrorIsNotSupportedInRustError,
        })
    }
}

type Result<T> = std::result::Result<T, Error>;
