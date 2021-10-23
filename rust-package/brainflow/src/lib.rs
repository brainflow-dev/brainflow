extern crate num;
#[macro_use]
extern crate num_derive;

use error::{Error};

pub mod board_ids;
pub mod agg_operations;
pub mod brainflow_classifiers;
pub mod detrend_operations;
pub mod error;
pub mod filter_types;
pub mod ip_protocol_type;
pub mod log_levels;
pub mod noise_types;
pub mod window_functions;
pub mod brainflow_exit_codes;
/// The primary interface to all boards.
pub mod board_shim;
/// Input parameters for [board_shim::BoardShim].
pub mod brainflow_input_params;

/// Methods for signal processig.
pub mod data_filter;
mod ffi;
/// Input parameters for [ml_model::MLModel].
pub mod brainflow_model_params;
/// Used to calculate derivative metrics from raw data.
pub mod ml_model;

/// Enum to store all supported Board Ids.
pub use board_ids::BoardIds;
/// Enum to store all supported Agg Operations.
pub use agg_operations::AggOperations;
/// Enum to store all supported BrainFlow Classifiers.
pub use brainflow_classifiers::BrainFlowClassifiers;
/// Enum to store all supported Detrend Operations.
pub use detrend_operations::DetrendOperations;
/// Store all supported BrainFlow Errors.
pub use error::BrainFlowError;
/// Enum to store all supported Filter Types.
pub use filter_types::FilterTypes;
/// Enum to store all supported Ip Protocol Types.
pub use ip_protocol_type::IpProtocolType;
/// Enum to store all supported Log Levels.
pub use log_levels::LogLevels;
/// Enum to store all supported Noise Types.
pub use noise_types::NoiseTypes;
/// Enum to store all supported Window Functions.
pub use window_functions::WindowFunctions;
/// Enum to store all Brainflow Exit Codes.
pub use brainflow_exit_codes::BrainFlowExitCodes;

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
