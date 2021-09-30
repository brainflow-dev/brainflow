use std::{
    ffi::CString,
    os::raw::{c_double, c_int},
    path::Path,
    sync::Mutex,
};

use crate::{check_brainflow_exit_code, Result};

mod brainflow_model_param;
pub use brainflow_model_param::{BrainFlowModelParams, BrainFlowModelParamsBuilder};

use brainflow_sys::constants::LogLevels;
use brainflow_sys::ml_model::MlModule;
use once_cell::sync::Lazy;

pub static ML_MODULE: Lazy<Mutex<MlModule>> = Lazy::new(|| {
    let lib_path = Path::new(
        "../brainflow-sys/target/debug/build/brainflow-sys-7b10940f08b63c04/out/lib/libMLModule.so",
    );
    let ml_module = unsafe { MlModule::new(lib_path).unwrap() };
    Mutex::new(ml_module)
});

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
        let res = unsafe {
            ML_MODULE
                .lock()
                .unwrap()
                .prepare(self.json_model_params.as_ptr())
        };
        Ok(check_brainflow_exit_code(res)?)
    }

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

pub fn set_log_level(log_level: LogLevels) -> Result<()> {
    let res = unsafe { ML_MODULE.lock().unwrap().set_log_level(log_level as c_int) };
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
    let res = unsafe { ML_MODULE.lock().unwrap().set_log_file(log_file.as_ptr()) };
    Ok(check_brainflow_exit_code(res)?)
}
