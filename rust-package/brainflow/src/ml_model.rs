use std::{
    ffi::CString,
    os::raw::{c_double, c_int},
};

use crate::{
    brainflow_model_params::BrainFlowModelParams, check_brainflow_exit_code, LogLevels, Result,
};

use crate::ffi::ml_model;

pub struct MlModel {
    json_model_params: CString,
}

impl MlModel {
    /// Create a new MlModel.
    pub fn new(model_params: BrainFlowModelParams) -> Result<Self> {
        let json_model_params = serde_json::to_string(&model_params)?;
        let json_model_params = CString::new(json_model_params)?;
        Ok(Self { json_model_params })
    }

    /// Prepare classifier.
    pub fn prepare(&self) -> Result<()> {
        let res = unsafe { ml_model::prepare(self.json_model_params.as_ptr()) };
        Ok(check_brainflow_exit_code(res)?)
    }

    /// Calculate metric from data.
    pub fn predict(&self, data: &mut [f64]) -> Result<f64> {
        let mut output = 0.0;
        let res = unsafe {
            ml_model::predict(
                data.as_mut_ptr() as *mut c_double,
                data.len() as c_int,
                &mut output,
                self.json_model_params.as_ptr(),
            )
        };
        check_brainflow_exit_code(res)?;
        Ok(output)
    }

    /// Release classifier.
    pub fn release(&self) -> Result<()> {
        let res = unsafe { ml_model::release(self.json_model_params.as_ptr()) };
        Ok(check_brainflow_exit_code(res)?)
    }
}

/// Set BrainFlow ML log level.
/// Use it only if you want to write your own messages to BrainFlow logger.
/// Otherwise use [enable_ml_logger], [enable_dev_ml_logger], or [disable_ml_logger].
pub fn set_log_level(log_level: LogLevels) -> Result<()> {
    let res = unsafe { ml_model::set_log_level_ml_module(log_level as c_int) };
    Ok(check_brainflow_exit_code(res)?)
}

/// Enable ML logger with level INFO, uses stderr for log messages by default.
pub fn enable_ml_logger() -> Result<()> {
    set_log_level(LogLevels::LevelInfo)
}

/// Disable BrainFlow ML logger.
pub fn disable_ml_logger() -> Result<()> {
    set_log_level(LogLevels::LevelOff)
}

/// Enable ML logger with level TRACE, uses stderr for log messages by default.
pub fn enable_dev_ml_logger() -> Result<()> {
    set_log_level(LogLevels::LevelTrace)
}

/// Redirect ML logger from stderr to file, can be called any time.
pub fn set_log_file<S: AsRef<str>>(log_file: S) -> Result<()> {
    let log_file = log_file.as_ref();
    let log_file = CString::new(log_file)?;
    let res = unsafe { ml_model::set_log_file_ml_module(log_file.as_ptr()) };
    Ok(check_brainflow_exit_code(res)?)
}
