use once_cell::sync::Lazy;
use paste::paste;
use std::{
    ffi::CString,
    mem,
    os::raw::{c_double, c_int},
    path::Path,
    sync::Mutex,
    vec,
};

use crate::{
    brainflow_input_params::BrainFlowInputParams,
    check_brainflow_exit_code,
    error::{BrainFlowError, Error},
    BoardId, Result,
};

use crate::ffi::board_controller::BoardController;
use crate::ffi::constants::LogLevels;

const MAX_CHANNELS: usize = 512;

#[cfg(target_os = "windows")]
pub static BOARD_CONTROLLER: Lazy<Mutex<BoardController>> = Lazy::new(|| {
    #[cfg(target_pointer_width = "64")]
    let lib_path = Path::new("lib\\libBoardController.dll");
    #[cfg(target_pointer_width = "32")]
    let lib_path = Path::new("lib\\libBoardController32.dll");
    let board_controller = unsafe { BoardController::new(lib_path).unwrap() };
    Mutex::new(board_controller)
});

#[cfg(target_os = "macos")]
pub static BOARD_CONTROLLER: Lazy<Mutex<BoardController>> = Lazy::new(|| {
    let lib_path = Path::new("lib/libBoardController.dylib");
    let board_controller = unsafe { BoardController::new(lib_path).unwrap() };
    Mutex::new(board_controller)
});

#[cfg(target_os = "linux")]
pub static BOARD_CONTROLLER: Lazy<Mutex<BoardController>> = Lazy::new(|| {
    let lib_path = Path::new("lib/libBoardController.so");
    let board_controller = unsafe { BoardController::new(lib_path).unwrap() };
    Mutex::new(board_controller)
});

/// BoardShim is a primary interface to all boards
pub struct BoardShim {
    board_id: BoardId,
    input_params: BrainFlowInputParams,
    json_brainflow_input_params: CString,
}

impl BoardShim {
    /// Creates a new [BoardShim].
    pub fn new(board_id: BoardId, input_params: BrainFlowInputParams) -> Result<Self> {
        let json_input_params = serde_json::to_string(&input_params)?;
        let json_input_params = CString::new(json_input_params)?;
        Ok(Self {
            board_id,
            input_params,
            json_brainflow_input_params: json_input_params,
        })
    }
    /// Prepare streaming sesssion and initialize resources.
    /// You need to call it before any other BoardShim object methods.
    pub fn prepare_session(&self) -> Result<()> {
        let res = unsafe {
            BOARD_CONTROLLER.lock().unwrap().prepare_session(
                self.board_id as c_int,
                self.json_brainflow_input_params.as_ptr(),
            )
        };
        Ok(check_brainflow_exit_code(res)?)
    }

    /// Returns true if the session is ready.
    pub fn is_prepared(&self) -> Result<bool> {
        let mut prepared = 0;
        let res = unsafe {
            BOARD_CONTROLLER.lock().unwrap().is_prepared(
                &mut prepared,
                self.board_id as c_int,
                self.json_brainflow_input_params.as_ptr(),
            )
        };
        check_brainflow_exit_code(res)?;
        Ok(prepared > 0)
    }

    /// Start streaming data, this methods stores data in ringbuffer.
    pub fn start_stream<S: AsRef<str>>(
        &self,
        buffer_size: usize,
        streamer_params: S,
    ) -> Result<()> {
        let streamer_params = CString::new(streamer_params.as_ref())?;
        let res = unsafe {
            BOARD_CONTROLLER.lock().unwrap().start_stream(
                buffer_size as c_int,
                streamer_params.as_ptr(),
                self.board_id as c_int,
                self.json_brainflow_input_params.as_ptr(),
            )
        };
        Ok(check_brainflow_exit_code(res)?)
    }

    /// Stop streaming data.
    pub fn stop_stream(&self) -> Result<()> {
        let res = unsafe {
            BOARD_CONTROLLER.lock().unwrap().stop_stream(
                self.board_id as c_int,
                self.json_brainflow_input_params.as_ptr(),
            )
        };
        Ok(check_brainflow_exit_code(res)?)
    }

    /// Release all resources.
    pub fn release_session(&self) -> Result<()> {
        let res = unsafe {
            BOARD_CONTROLLER.lock().unwrap().release_session(
                self.board_id as c_int,
                self.json_brainflow_input_params.as_ptr(),
            )
        };
        Ok(check_brainflow_exit_code(res)?)
    }

    /// Get num of elements in ringbuffer.
    pub fn get_board_data_count(&self) -> Result<usize> {
        let mut data_count = 0;
        let res = unsafe {
            BOARD_CONTROLLER.lock().unwrap().get_board_data_count(
                &mut data_count,
                self.board_id as c_int,
                self.json_brainflow_input_params.as_ptr(),
            )
        };
        check_brainflow_exit_code(res)?;
        Ok(data_count as usize)
    }

    /// Get board data and remove data from ringbuffer
    pub fn get_board_data(&self, n_data_points: Option<usize>) -> Result<Vec<Vec<f64>>> {
        let num_rows = get_num_rows(self.board_id)?;
        let num_samples = if let Some(n) = n_data_points {
            self.get_board_data_count()?.min(n)
        } else {
            self.get_board_data_count()?
        };

        let capacity = num_samples * num_rows;
        let mut data_buf = Vec::with_capacity(capacity);
        let res = unsafe {
            BOARD_CONTROLLER.lock().unwrap().get_board_data(
                num_samples as c_int,
                data_buf.as_mut_ptr(),
                self.board_id as c_int,
                self.json_brainflow_input_params.as_ptr(),
            )
        };
        check_brainflow_exit_code(res)?;

        unsafe { data_buf.set_len(capacity) };
        Ok(data_buf.chunks(num_samples).map(|d| d.to_vec()).collect())
    }

    /// Get specified amount of data or less if there is not enough data, doesnt remove data from ringbuffer.
    pub fn get_current_board_data(&self, num_samples: usize) -> Result<Vec<Vec<f64>>> {
        let num_rows = get_num_rows(self.board_id)?;
        let capacity = num_samples * num_rows;
        let mut data_buf = Vec::with_capacity(capacity);
        let res = unsafe {
            BOARD_CONTROLLER.lock().unwrap().get_current_board_data(
                num_samples as c_int,
                data_buf.as_mut_ptr(),
                &mut 0,
                self.board_id as c_int,
                self.json_brainflow_input_params.as_ptr(),
            )
        };
        check_brainflow_exit_code(res)?;

        unsafe { data_buf.set_len(capacity) };
        Ok(data_buf.chunks(num_samples).map(|d| d.to_vec()).collect())
    }

    /// Use this method carefully and only if you understand what you are doing, do NOT use it to start or stop streaming
    pub fn config_board<S: AsRef<str>>(&self, config: S) -> Result<String> {
        let config = CString::new(config.as_ref())?;
        let mut response_len = 0;
        let response = CString::new(Vec::with_capacity(8192))?;
        let response = response.into_raw();
        let config = config.into_raw();
        let (res, response) = unsafe {
            let res = BOARD_CONTROLLER.lock().unwrap().config_board(
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

    /// Insert Marker to Data Stream.
    pub fn insert_marker(&self, value: f64) -> Result<()> {
        let res = unsafe {
            BOARD_CONTROLLER.lock().unwrap().insert_marker(
                value as c_double,
                self.board_id as c_int,
                self.json_brainflow_input_params.as_ptr(),
            )
        };
        Ok(check_brainflow_exit_code(res)?)
    }

    /// Get's the actual board id, can be different than provided.
    pub fn get_board_id(&self) -> Result<BoardId> {
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

/// Set BrainFlow log level, use it only if you want to write your own messages to BrainFlow logger,
/// otherwise use [enable_board_logger], [enable_dev_board_logger] or [disable_board_logger].
pub fn set_log_level(log_level: LogLevels) -> Result<()> {
    let res = unsafe {
        BOARD_CONTROLLER
            .lock()
            .unwrap()
            .set_log_level(log_level as c_int)
    };
    Ok(check_brainflow_exit_code(res)?)
}

/// Enable BrainFlow board logger with level INFO, uses stderr for log messages by default.
pub fn enable_board_logger() -> Result<()> {
    set_log_level(LogLevels::LEVEL_INFO)
}

/// Disable BrainFlow board logger.
pub fn disable_board_logger() -> Result<()> {
    set_log_level(LogLevels::LEVEL_OFF)
}

/// Enable BrainFlow board logger with level TRACE, uses stderr for log messages by default.
pub fn enable_dev_board_logger() -> Result<()> {
    set_log_level(LogLevels::LEVEL_TRACE)
}

/// Redirect board logger from stderr to file, can be called any time.
pub fn set_log_file<S: AsRef<str>>(log_file: S) -> Result<()> {
    let log_file = log_file.as_ref();
    let log_file = CString::new(log_file)?;
    let res = unsafe {
        BOARD_CONTROLLER
            .lock()
            .unwrap()
            .set_log_file(log_file.as_ptr())
    };
    Ok(check_brainflow_exit_code(res)?)
}

macro_rules! gen_fn {
    ($fn_name:ident, $return_type:ident, $initial_value:literal, $doc:literal) => {
        paste! {
            #[doc = $doc]
            pub fn [<get_$fn_name>]( board_id: BoardId) -> Result<$return_type> {
                let mut value = $initial_value;
                let res = unsafe { BOARD_CONTROLLER.lock().unwrap().[<get_$fn_name>](board_id as c_int, &mut value) };
                check_brainflow_exit_code(res)?;
                Ok(value as $return_type)
            }
        }
    };
}

gen_fn!(sampling_rate, isize, -1, "Write your own log message to BrainFlow logger, use it if you wanna have single logger for your own code and BrainFlow's code.");
gen_fn!(
    package_num_channel,
    isize,
    -1,
    "Get package num channel for a board."
);
gen_fn!(
    timestamp_channel,
    isize,
    0,
    "Get timestamp channel in resulting data table for a board."
);
gen_fn!(
    marker_channel,
    isize,
    0,
    "Get marker channel in resulting data table for a board."
);
gen_fn!(
    battery_channel,
    isize,
    0,
    "Get battery channel for a board."
);
gen_fn!(
    num_rows,
    usize,
    0,
    "Get number of rows in resulting data table for a board."
);

/// Write your own log message to BrainFlow board logger, use it if you wanna have single logger for your own code and BrainFlow's code.
pub fn log_message<S: AsRef<str>>(log_level: LogLevels, message: S) -> Result<()> {
    let message = message.as_ref();
    let message = CString::new(message)?.into_raw();
    let res = unsafe {
        let res = BOARD_CONTROLLER
            .lock()
            .unwrap()
            .log_message(log_level as c_int, message);
        let _ = CString::from_raw(message);
        res
    };
    Ok(check_brainflow_exit_code(res)?)
}

/// Get board description as json.
pub fn get_board_descr(board_id: BoardId) -> Result<String> {
    let mut response_len = 0;
    let response = CString::new(Vec::with_capacity(16000))?;
    let response = response.into_raw();
    let (res, response) = unsafe {
        let res = BOARD_CONTROLLER.lock().unwrap().get_board_descr(
            board_id as c_int,
            response,
            &mut response_len,
        );
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

/// Get names of EEG channels in 10-20 system if their location is fixed.
pub fn get_eeg_names(board_id: BoardId) -> Result<Vec<String>> {
    let mut response_len = 0;
    let response = CString::new(Vec::with_capacity(16000))?;
    let response = response.into_raw();
    let (res, response) = unsafe {
        let res = BOARD_CONTROLLER.lock().unwrap().get_eeg_names(
            board_id as c_int,
            response,
            &mut response_len,
        );
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

/// Get device name.
pub fn get_device_name(board_id: BoardId) -> Result<String> {
    let mut response_len = 0;
    let response = CString::new(Vec::with_capacity(4096))?;
    let response = response.into_raw();
    let (res, response) = unsafe {
        let res = BOARD_CONTROLLER.lock().unwrap().get_device_name(
            board_id as c_int,
            response,
            &mut response_len,
        );
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
    ($fn_name:ident, $doc:literal) => {
        paste! {
            #[doc = $doc]
            pub fn [<get_$fn_name>](board_id: BoardId) -> Result<Vec<isize>> {
                let mut channels: Vec<isize> = Vec::with_capacity(MAX_CHANNELS);
                let channels_ptr = channels.as_mut_ptr();
                let mut len = 0;
                let res = unsafe {
                    mem::forget(channels);
                    BOARD_CONTROLLER.lock().unwrap().[<get_$fn_name>](
                        board_id as c_int,
                        channels_ptr as *mut c_int,
                        &mut len,
                    )
                };
                check_brainflow_exit_code(res)?;
                let channels: Vec<isize> = unsafe {Vec::from_raw_parts(channels_ptr, len as usize, MAX_CHANNELS)};
                Ok(channels)
            }
        }
    };
}

gen_vec_fn!(
    eeg_channels,
    "Get list of eeg channels in resulting data table for a board."
);
gen_vec_fn!(
    exg_channels,
    "Get list of exg channels in resulting data table for a board."
);
gen_vec_fn!(
    emg_channels,
    "Get list of emg channels in resulting data table for a board."
);
gen_vec_fn!(
    ecg_channels,
    "Get list of ecg channels in resulting data table for a board."
);
gen_vec_fn!(
    eog_channels,
    "Get list of eog channels in resulting data table for a board."
);
gen_vec_fn!(
    eda_channels,
    "Get list of eda channels in resulting data table for a board."
);
gen_vec_fn!(
    ppg_channels,
    "Get list of ppg channels in resulting data table for a board."
);
gen_vec_fn!(
    accel_channels,
    "Get list of accel channels in resulting data table for a board."
);
gen_vec_fn!(
    gyro_channels,
    "Get list of gyro channels in resulting data table for a board."
);
gen_vec_fn!(
    analog_channels,
    "Get list of analog channels in resulting data table for a board."
);
gen_vec_fn!(
    other_channels,
    "Get list of other channels in resulting data table for a board."
);
gen_vec_fn!(
    temperature_channels,
    "Get list of temperature channels in resulting data table for a board."
);
gen_vec_fn!(
    resistance_channels,
    "Get list of resistance channels in resulting data table for a board."
);
