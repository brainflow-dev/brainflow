use std::{ffi::CString, os::raw::c_int};

use crate::{brainflow_input_params::BrainFlowInputParams, check_brainflow_exit_code, ffi, Result};

pub struct BoardShim {
    board_id: ffi::BoardIds,
    input_params: BrainFlowInputParams,
    json_input_params: CString,
}

impl BoardShim {
    pub fn new(board_id: ffi::BoardIds, input_params: BrainFlowInputParams) -> Result<Self> {
        let json_input_params = serde_json::to_string(&input_params)?;
        let json_input_params = CString::new(json_input_params)?;
        Ok(Self {
            board_id,
            input_params,
            json_input_params,
        })
    }

    pub fn prepare_session(&self) -> Result<()> {
        let res = unsafe {
            ffi::prepare_session(self.board_id as c_int, self.json_input_params.as_ptr())
        };
        Ok(check_brainflow_exit_code(res)?)
    }

    pub fn is_prepared(&self) -> Result<bool> {
        let prepared: c_int = 0;
        let res = unsafe {
            ffi::is_prepared(
                prepared as *mut c_int,
                self.board_id as c_int,
                self.json_input_params.as_ptr(),
            )
        };
        check_brainflow_exit_code(res)?;
        Ok(prepared > 0)
    }
}

pub fn set_log_level(log_level: ffi::LogLevels) -> Result<()> {
    let res = unsafe { ffi::set_log_level(log_level as c_int) };
    Ok(check_brainflow_exit_code(res)?)
}

pub fn enable_board_logger() -> Result<()> {
    set_log_level(ffi::LogLevels::LEVEL_INFO)
}

pub fn disable_board_logger() -> Result<()> {
    set_log_level(ffi::LogLevels::LEVEL_OFF)
}

pub fn enable_dev_board_logger() -> Result<()> {
    set_log_level(ffi::LogLevels::LEVEL_TRACE)
}

pub fn set_log_file<S: AsRef<str>>(log_file: S) -> Result<()> {
    let log_file = log_file.as_ref();
    let log_file = CString::new(log_file)?;
    let res = unsafe { ffi::set_log_file(log_file.as_ptr()) };
    Ok(check_brainflow_exit_code(res)?)
}

pub fn log_message<S: AsRef<str>>(log_level: ffi::LogLevels, message: S) -> Result<()> {
    let message = message.as_ref();
    let message = CString::new(message)?;
    let res = unsafe { ffi::log_message(log_level as c_int, message.into_raw()) };
    Ok(check_brainflow_exit_code(res)?)
}
