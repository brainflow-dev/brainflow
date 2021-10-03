use std::{
    ffi::CString,
    os::raw::{c_double, c_int},
    path::Path,
    sync::Mutex,
};

use crate::{check_brainflow_exit_code, Result};

mod brainflow_model_param;
pub use brainflow_model_param::{BrainFlowModelParams, BrainFlowModelParamsBuilder};

use crate::ffi::constants::LogLevels;
use crate::ffi::ml_model::MlModule;
use once_cell::sync::Lazy;

#[cfg(target_os = "windows")]
pub static ML_MODULE: Lazy<Mutex<MlModule>> = Lazy::new(|| {
    #[cfg(target_pointer_width = "64")]
    let lib_path = Path::new("lib\\libMLModule.dll");
    #[cfg(target_pointer_width = "32")]
    let lib_path = Path::new("lib\\libMLModule32.dll");
    let ml_module = unsafe { MlModule::new(lib_path).unwrap() };
    Mutex::new(ml_module)
});

#[cfg(target_os = "macos")]
pub static ML_MODULE: Lazy<Mutex<MlModule>> = Lazy::new(|| {
    let lib_path = Path::new("lib/libMLModule.dylib");
    let ml_module = unsafe { MlModule::new(lib_path).unwrap() };
    Mutex::new(ml_module)
});

#[cfg(target_os = "linux")]
pub static ML_MODULE: Lazy<Mutex<MlModule>> = Lazy::new(|| {
    let lib_path = Path::new("lib/libMLModule.so");
    let ml_module = unsafe { MlModule::new(lib_path).unwrap() };
    Mutex::new(ml_module)
});

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
        let res = unsafe {
            ML_MODULE
                .lock()
                .unwrap()
                .prepare(self.json_model_params.as_ptr())
        };
        Ok(check_brainflow_exit_code(res)?)
    }

    /// Calculate metric from data.
    pub fn predict(&self, data: &mut [f64]) -> Result<f64> {
        let mut output = 0.0;
        let res = unsafe {
            ML_MODULE.lock().unwrap().predict(
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
        let res = unsafe {
            ML_MODULE
                .lock()
                .unwrap()
                .release(self.json_model_params.as_ptr())
        };
        Ok(check_brainflow_exit_code(res)?)
    }
}

/// Set BrainFlow ML log level.
/// Use it only if you want to write your own messages to BrainFlow logger.
/// Otherwise use [enable_ml_logger], [enable_dev_ml_logger], or [disable_ml_logger].
pub fn set_log_level(log_level: LogLevels) -> Result<()> {
    let res = unsafe { ML_MODULE.lock().unwrap().set_log_level(log_level as c_int) };
    Ok(check_brainflow_exit_code(res)?)
}

/// Enable ML logger with level INFO, uses stderr for log messages by default.
pub fn enable_ml_logger() -> Result<()> {
    set_log_level(LogLevels::LEVEL_INFO)
}

/// Disable BrainFlow ML logger.
pub fn disable_ml_logger() -> Result<()> {
    set_log_level(LogLevels::LEVEL_OFF)
}

/// Enable ML logger with level TRACE, uses stderr for log messages by default.
pub fn enable_dev_ml_logger() -> Result<()> {
    set_log_level(LogLevels::LEVEL_TRACE)
}

/// Redirect ML logger from stderr to file, can be called any time.
pub fn set_log_file<S: AsRef<str>>(log_file: S) -> Result<()> {
    let log_file = log_file.as_ref();
    let log_file = CString::new(log_file)?;
    let res = unsafe { ML_MODULE.lock().unwrap().set_log_file(log_file.as_ptr()) };
    Ok(check_brainflow_exit_code(res)?)
}
