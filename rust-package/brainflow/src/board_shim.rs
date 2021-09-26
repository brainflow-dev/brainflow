use paste::paste;
use std::{
    ffi::CString,
    os::raw::{c_double, c_int},
};

use ffi::LogLevels;

use crate::{
    brainflow_input_params::BrainFlowInputParams,
    check_brainflow_exit_code,
    error::{BrainFlowError, Error},
    ffi, BoardId, Result,
};

const MAX_CHANNELS: usize = 512;

pub struct BoardShim {
    board_id: BoardId,
    input_params: BrainFlowInputParams,
    json_brainflow_input_params: CString,
}

impl BoardShim {
    pub fn new(board_id: BoardId, input_params: BrainFlowInputParams) -> Result<Self> {
        dbg!(&input_params);
        let json_input_params = serde_json::to_string(&input_params)?;
        let json_input_params = CString::new(json_input_params)?;
        Ok(Self {
            board_id,
            input_params,
            json_brainflow_input_params: json_input_params,
        })
    }

    pub fn prepare_session(&self) -> Result<()> {
        let res = unsafe {
            ffi::prepare_session(
                self.board_id as c_int,
                self.json_brainflow_input_params.as_ptr(),
            )
        };
        Ok(check_brainflow_exit_code(res)?)
    }

    pub fn is_prepared(&self) -> Result<bool> {
        let mut prepared = 0;
        let res = unsafe {
            ffi::is_prepared(
                &mut prepared,
                self.board_id as c_int,
                self.json_brainflow_input_params.as_ptr(),
            )
        };
        check_brainflow_exit_code(res)?;
        Ok(prepared > 0)
    }

    pub fn start_stream<S: AsRef<str>>(
        &self,
        buffer_size: usize,
        streamer_params: S,
    ) -> Result<()> {
        let streamer_params = CString::new(streamer_params.as_ref())?;
        let res = unsafe {
            ffi::start_stream(
                buffer_size as c_int,
                streamer_params.as_ptr(),
                self.board_id as c_int,
                self.json_brainflow_input_params.as_ptr(),
            )
        };
        Ok(check_brainflow_exit_code(res)?)
    }

    pub fn stop_stream(&self) -> Result<()> {
        let res = unsafe {
            ffi::stop_stream(
                self.board_id as c_int,
                self.json_brainflow_input_params.as_ptr(),
            )
        };
        Ok(check_brainflow_exit_code(res)?)
    }

    pub fn release_session(&self) -> Result<()> {
        let res = unsafe {
            ffi::release_session(
                self.board_id as c_int,
                self.json_brainflow_input_params.as_ptr(),
            )
        };
        Ok(check_brainflow_exit_code(res)?)
    }

    pub fn board_data_count(&self) -> Result<usize> {
        let mut data_count = 0;
        let res = unsafe {
            ffi::get_board_data_count(
                &mut data_count,
                self.board_id as c_int,
                self.json_brainflow_input_params.as_ptr(),
            )
        };
        check_brainflow_exit_code(res)?;
        Ok(data_count as usize)
    }

    pub fn get_board_data_count(&self) -> Result<usize> {
        self.board_data_count()
    }

    pub fn board_data(&self, n_data_points: Option<usize>) -> Result<Vec<Vec<f64>>> {
        let num_rows = num_rows(self.board_id)?;
        let num_samples = if let Some(n) = n_data_points {
            self.board_data_count()?.min(n)
        } else {
            self.board_data_count()?
        };
        let mut data_buf = Vec::with_capacity(num_samples * num_rows);
        let res = unsafe {
            ffi::get_board_data(
                num_samples as c_int,
                data_buf.as_mut_ptr(),
                self.board_id as c_int,
                self.json_brainflow_input_params.as_ptr(),
            )
        };
        check_brainflow_exit_code(res)?;

        let data_buf = data_buf.chunks(num_samples).map(|d| d.to_vec()).collect();
        Ok(data_buf)
    }

    pub fn get_board_data(&self, n_data_points: Option<usize>) -> Result<Vec<Vec<f64>>> {
        self.board_data(n_data_points)
    }

    pub fn current_board_data(&self, num_samples: usize) -> Result<Vec<Vec<f64>>> {
        let num_rows = num_rows(self.board_id)?;
        let mut data_buf = Vec::with_capacity(num_samples * num_rows);
        let mut len = 0;
        let res = unsafe {
            ffi::get_current_board_data(
                num_samples as c_int,
                data_buf.as_mut_ptr(),
                &mut len,
                self.board_id as c_int,
                self.json_brainflow_input_params.as_ptr(),
            )
        };
        check_brainflow_exit_code(res)?;

        let data_buf = data_buf.chunks(num_samples).map(|d| d.to_vec()).collect();
        Ok(data_buf)
    }

    pub fn get_current_board_data(&self, num_samples: usize) -> Result<Vec<Vec<f64>>> {
        self.current_board_data(num_samples)
    }

    pub fn config_board<S: AsRef<str>>(&self, config: S) -> Result<String> {
        let config = CString::new(config.as_ref())?;
        let mut response_len = 0;
        let response = CString::new(Vec::with_capacity(8192))?;
        let response = response.into_raw();
        let config = config.into_raw();
        let (res, response) = unsafe {
            let res = ffi::config_board(
                config,
                response,
                &mut response_len,
                self.board_id as c_int,
                self.json_brainflow_input_params.as_ptr(),
            );
            let _ = CString::from_raw(config);
            let response = CString::from_raw(response);
            (res, response)
        };
        check_brainflow_exit_code(res)?;
        Ok(response
            .to_str()?
            .split_at(response_len as usize)
            .0
            .to_string())
    }

    pub fn insert_marker(&self, value: f64) -> Result<()> {
        let res = unsafe {
            ffi::insert_marker(
                value as c_double,
                self.board_id as c_int,
                self.json_brainflow_input_params.as_ptr(),
            )
        };
        Ok(check_brainflow_exit_code(res)?)
    }

    pub fn board_id(&self) -> Result<BoardId> {
        Ok(match &self.board_id {
            BoardId::StreamingBoard | BoardId::PlaybackFileBoard => {
                let id = self.input_params.other_info().parse::<i32>().unwrap();
                num::FromPrimitive::from_i32(id)
                    .ok_or(Error::BrainFlowError(BrainFlowError::InvalidArgumentsError))?
            }
            _ => self.board_id,
        })
    }
}

pub fn set_log_level(log_level: LogLevels) -> Result<()> {
    let res = unsafe { ffi::set_log_level(log_level as c_int) };
    Ok(check_brainflow_exit_code(res)?)
}

pub fn enable_board_logger() -> Result<()> {
    set_log_level(LogLevels::LEVEL_INFO)
}

pub fn disable_board_logger() -> Result<()> {
    set_log_level(LogLevels::LEVEL_OFF)
}

pub fn enable_dev_board_logger() -> Result<()> {
    set_log_level(LogLevels::LEVEL_TRACE)
}

pub fn set_log_file<S: AsRef<str>>(log_file: S) -> Result<()> {
    let log_file = log_file.as_ref();
    let log_file = CString::new(log_file)?;
    let res = unsafe { ffi::set_log_file(log_file.as_ptr()) };
    Ok(check_brainflow_exit_code(res)?)
}

pub fn log_message<S: AsRef<str>>(log_level: LogLevels, message: S) -> Result<()> {
    let message = message.as_ref();
    let message = CString::new(message)?.into_raw();
    let res = unsafe {
        let res = ffi::log_message(log_level as c_int, message);
        let _ = CString::from_raw(message);
        res
    };
    Ok(check_brainflow_exit_code(res)?)
}

pub fn board_descr(board_id: BoardId) -> Result<String> {
    let mut response_len = 0;
    let response = CString::new(Vec::with_capacity(16000))?;
    let response = response.into_raw();
    let (res, response) = unsafe {
        let res = ffi::get_board_descr(board_id as c_int, response, &mut response_len);
        let response = CString::from_raw(response);
        (res, response)
    };
    check_brainflow_exit_code(res)?;
    Ok(response
        .to_str()?
        .split_at(response_len as usize)
        .0
        .to_string())
}

pub fn get_board_descr(board_id: BoardId) -> Result<String> {
    board_descr(board_id)
}

macro_rules! gen_fn {
    ($fn_name:ident, $return_type:ident, $initial_value:literal) => {
        paste! {
            pub fn $fn_name(board_id: BoardId) -> Result<$return_type> {
                let mut value = $initial_value;
                let res = unsafe { ffi::[<get_$fn_name>](board_id as c_int, &mut value) };
                check_brainflow_exit_code(res)?;
                Ok(value as $return_type)
            }

            pub fn [<get_$fn_name>](board_id: BoardId) -> Result<$return_type> {
                $fn_name(board_id)
            }
        }
    };
}

gen_fn!(sampling_rate, isize, -1);
gen_fn!(package_num_channel, isize, -1);
gen_fn!(timestamp_channel, isize, 0);
gen_fn!(marker_channel, isize, 0);
gen_fn!(battery_channel, isize, 0);
gen_fn!(num_rows, usize, 0);

pub fn eeg_names(board_id: BoardId) -> Result<Vec<String>> {
    let mut response_len = 0;
    let response = CString::new(Vec::with_capacity(16000))?;
    let response = response.into_raw();
    let (res, response) = unsafe {
        let res = ffi::get_eeg_names(board_id as c_int, response, &mut response_len);
        let response = CString::from_raw(response);
        (res, response)
    };
    check_brainflow_exit_code(res)?;
    let names = response.to_str()?.split_at(response_len as usize).0;

    Ok(names
        .split(',')
        .map(|s| s.to_string())
        .collect::<Vec<String>>())
}

pub fn get_eeg_names(board_id: BoardId) -> Result<Vec<String>> {
    eeg_names(board_id)
}

pub fn device_name(board_id: BoardId) -> Result<String> {
    let mut response_len = 0;
    let response = CString::new(Vec::with_capacity(4096))?;
    let response = response.into_raw();
    let (res, response) = unsafe {
        let res = ffi::get_device_name(board_id as c_int, response, &mut response_len);
        let response = CString::from_raw(response);
        (res, response)
    };
    check_brainflow_exit_code(res)?;
    Ok(response
        .to_str()?
        .split_at(response_len as usize)
        .0
        .to_string())
}

macro_rules! gen_vec_fn {
    ($fn_name:ident) => {
        paste! {
            pub fn $fn_name(board_id: BoardId) -> Result<Vec<isize>> {
                let mut channels: Vec<isize> = Vec::with_capacity(MAX_CHANNELS);
                let mut len = 0;
                let res = unsafe {
                    ffi::[<get_$fn_name>](
                        board_id as c_int,
                        channels.as_mut_ptr() as *mut c_int,
                        &mut len,
                    )
                };
                check_brainflow_exit_code(res)?;
                channels.resize(len as usize, 0);
                Ok(channels)
            }

            pub fn [<get_$fn_name>](board_id: BoardId) -> Result<Vec<isize>> {
                $fn_name(board_id)
            }
        }
    };
}

gen_vec_fn!(eeg_channels);
gen_vec_fn!(exg_channels);
gen_vec_fn!(emg_channels);
gen_vec_fn!(ecg_channels);
gen_vec_fn!(eog_channels);
gen_vec_fn!(eda_channels);
gen_vec_fn!(ppg_channels);
gen_vec_fn!(accel_channels);
gen_vec_fn!(gyro_channels);
gen_vec_fn!(analog_channels);
gen_vec_fn!(other_channels);
gen_vec_fn!(temperature_channels);
gen_vec_fn!(resistance_channels);
