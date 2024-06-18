use std::{
    ffi::{CString, CStr},
    os::raw::{c_double, c_int, c_char},
};

use crate::{
    brainflow_model_params::BrainFlowModelParams, check_brainflow_exit_code, LogLevels, Result,
};

use crate::ffi::ml_module;

pub struct MlModel {
    model_params: BrainFlowModelParams,
    json_model_params: CString,
}

impl MlModel {
    /// Create a new MlModel.
    pub fn new(model_params: BrainFlowModelParams) -> Result<Self> {
        let json_model_params = serde_json::to_string(&model_params)?;
        let json_model_params = CString::new(json_model_params)?;
        Ok(Self { model_params, json_model_params })
    }

    /// Prepare classifier.
    pub fn prepare(&self) -> Result<()> {
        let res = unsafe { ml_module::prepare(self.json_model_params.as_ptr()) };
        Ok(check_brainflow_exit_code(res)?)
    }

    /// Calculate metric from data.
    pub fn predict(&self, data: &mut [f64]) -> Result<Vec<f64>> {
        let mut output: Vec<f64> = Vec::with_capacity(*self.model_params.max_array_size());
        let mut output_len = 0;
        let res = unsafe {
            ml_module::predict(
                data.as_mut_ptr() as *mut c_double,
                data.len() as c_int,
                output.as_mut_ptr(),
                &mut output_len,
                self.json_model_params.as_ptr(),
            )
        };
        check_brainflow_exit_code(res)?;
        unsafe { output.set_len(output_len as usize) };
        let output_casted = output.into_iter().map(|c| c as f64).collect::<Vec<f64>>();
        Ok(output_casted)
    }

    /// Release classifier.
    pub fn release(&self) -> Result<()> {
        let res = unsafe { ml_module::release(self.json_model_params.as_ptr()) };
        Ok(check_brainflow_exit_code(res)?)
    }
}

/// Set BrainFlow ML log level.
/// Use it only if you want to write your own messages to BrainFlow logger.
/// Otherwise use [enable_ml_logger], [enable_dev_ml_logger], or [disable_ml_logger].
pub fn set_log_level(log_level: LogLevels) -> Result<()> {
    let res = unsafe { ml_module::set_log_level_ml_module(log_level as c_int) };
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
    let res = unsafe { ml_module::set_log_file_ml_module(log_file.as_ptr()) };
    Ok(check_brainflow_exit_code(res)?)
}

/// Write your own log message to BrainFlow board logger, use it if you wanna have single logger for your own code and BrainFlow's code.
pub fn log_message<S: AsRef<str>>(log_level: LogLevels, message: S) -> Result<()> {
    let message = message.as_ref();
    let message = CString::new(message)?.into_raw();
    let res = unsafe {
        let res = ml_module::log_message_ml_module(log_level as c_int, message);
        let _ = CString::from_raw(message);
        res
    };
    Ok(check_brainflow_exit_code(res)?)
}

/// Release all classifiers
pub fn release_all() -> Result<()> {
    let res = unsafe { ml_module::release_all() };
    Ok(check_brainflow_exit_code(res)?)
}

/// Get DataFilter version.
pub fn get_version() -> Result<String> {
    const MAX_CHARS: usize = 64;
    let mut response_len = 0;
    let mut result_char_buffer: [c_char; MAX_CHARS] = [0; MAX_CHARS];
    let (res, response) = unsafe {
        let res = ml_module::get_version_ml_module(result_char_buffer.as_mut_ptr(), &mut response_len, MAX_CHARS as i32);
        let response = CStr::from_ptr(result_char_buffer.as_ptr());
        (res, response)
    };
    check_brainflow_exit_code(res)?;
    Ok(response.to_str()?.to_string())
}

#[cfg(test)]
mod tests {
    use crate::ml_model::get_version;
    use crate::test_helpers::assertions::assert_regex_matches;
    use crate::test_helpers::consts::VERSION_PATTERN;

    #[test]
    fn test_it_gets_the_version() {
        assert_regex_matches(VERSION_PATTERN, get_version().unwrap().as_str());
    }
}
