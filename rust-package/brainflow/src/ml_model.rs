use std::{
    ffi::CString,
    os::raw::{c_double, c_int},
};

use crate::{check_brainflow_exit_code, ffi, Result};

mod brainflow_model_param;
pub use brainflow_model_param::{BrainFlowModelParams, BrainFlowModelParamsBuilder};
use ffi::LogLevels;

pub fn set_log_level(log_level: LogLevels) -> Result<()> {
    let res = unsafe { ffi::set_log_level(log_level as c_int) };
    Ok(check_brainflow_exit_code(res)?)
}

pub fn enable_ml_logger() -> Result<()> {
    set_log_level(LogLevels::LEVEL_INFO)
}

pub fn disable_ml_logger() -> Result<()> {
    set_log_level(LogLevels::LEVEL_OFF)
}

pub fn enable_dev_ml_logger() -> Result<()> {
    set_log_level(LogLevels::LEVEL_TRACE)
}

pub fn set_log_file<S: AsRef<str>>(log_file: S) -> Result<()> {
    let log_file = log_file.as_ref();
    let log_file = CString::new(log_file)?;
    let res = unsafe { ffi::set_log_file(log_file.as_ptr()) };
    Ok(check_brainflow_exit_code(res)?)
}

pub struct MlModel {
    model_params: BrainFlowModelParams,
    json_model_params: CString,
}

impl MlModel {
    pub fn new(model_params: BrainFlowModelParams) -> Result<Self> {
        let json_model_params = serde_json::to_string(&model_params)?;
        let json_model_params = CString::new(json_model_params)?;
        Ok(Self {
            model_params,
            json_model_params,
        })
    }

    pub fn prepare(&self) -> Result<()> {
        let res = unsafe { ffi::prepare(self.json_model_params.as_ptr()) };
        Ok(check_brainflow_exit_code(res)?)
    }

    pub fn predict(&self, data: &mut [f64]) -> Result<f64> {
        let mut output = 0.0;
        let res = unsafe {
            ffi::predict(
                data.as_mut_ptr() as *mut c_double,
                data.len() as c_int,
                &mut output,
                self.json_model_params.as_ptr(),
            )
        };
        check_brainflow_exit_code(res)?;
        Ok(output)
    }

    pub fn release(&self) -> Result<()> {
        let res = unsafe { ffi::release(self.json_model_params.as_ptr()) };
        Ok(check_brainflow_exit_code(res)?)
    }
}
