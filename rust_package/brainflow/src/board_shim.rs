use ndarray::Array2;
use paste::paste;
use std::{
    ffi::CString,
    ffi::CStr,
    os::raw::{c_double, c_int},
};
use std::os::raw::c_char;

use crate::{
    brainflow_input_params::BrainFlowInputParams, check_brainflow_exit_code, BoardIds, LogLevels,
    Result, BrainFlowPresets,
};

use crate::ffi::board_controller;

const MAX_CHANNELS: usize = 512;

/// BoardShim is a primary interface to all boards
pub struct BoardShim {
    board_id: BoardIds,
    master_board_id: BoardIds,
    json_brainflow_input_params: CString,
}

impl BoardShim {
    /// Creates a new [BoardShim].
    pub fn new(board_id: BoardIds, input_params: BrainFlowInputParams) -> Result<Self> {
        let json_brainflow_input_params = serde_json::to_string(&input_params)?;
        let json_brainflow_input_params = CString::new(json_brainflow_input_params)?;
        let master_board_id =
            if let BoardIds::StreamingBoard | BoardIds::PlaybackFileBoard = board_id {
                num::FromPrimitive::from_usize(*input_params.master_board()).unwrap()
            } else {
                board_id
            };
        Ok(Self {
            board_id,
            master_board_id,
            json_brainflow_input_params,
        })
    }

    /// Returns the currently used BrainFlowInputParams
    pub fn input_params(&self) -> Result<BrainFlowInputParams> {
        Ok(serde_json::from_str(
            self.json_brainflow_input_params.to_str().unwrap(),
        )?)
    }

    /// Prepare streaming sesssion and initialize resources.
    /// You need to call it before any other BoardShim object methods.
    pub fn prepare_session(&self) -> Result<()> {
        let res = unsafe {
            board_controller::prepare_session(
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
            board_controller::is_prepared(
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
            board_controller::start_stream(
                buffer_size as c_int,
                streamer_params.as_ptr(),
                self.board_id as c_int,
                self.json_brainflow_input_params.as_ptr(),
            )
        };
        Ok(check_brainflow_exit_code(res)?)
    }

    /// Add streamer from BrainFlow to file or streaming board.
    pub fn add_streamer<S: AsRef<str>>(
        &self,
        streamer_params: S,
        preset: BrainFlowPresets,
    ) -> Result<()> {
        let streamer_params = CString::new(streamer_params.as_ref())?;
        let res = unsafe {
            board_controller::add_streamer(
                streamer_params.as_ptr(),
                preset as c_int,
                self.board_id as c_int,
                self.json_brainflow_input_params.as_ptr(),
            )
        };
        Ok(check_brainflow_exit_code(res)?)
    }

    /// Delete streamer registered streamer.
    pub fn delete_streamer<S: AsRef<str>>(
        &self,
        streamer_params: S,
        preset: BrainFlowPresets,
    ) -> Result<()> {
        let streamer_params = CString::new(streamer_params.as_ref())?;
        let res = unsafe {
            board_controller::delete_streamer(
                streamer_params.as_ptr(),
                preset as c_int,
                self.board_id as c_int,
                self.json_brainflow_input_params.as_ptr(),
            )
        };
        Ok(check_brainflow_exit_code(res)?)
    }

    /// Stop streaming data.
    pub fn stop_stream(&self) -> Result<()> {
        let res = unsafe {
            board_controller::stop_stream(
                self.board_id as c_int,
                self.json_brainflow_input_params.as_ptr(),
            )
        };
        Ok(check_brainflow_exit_code(res)?)
    }

    /// Release all resources.
    pub fn release_session(&self) -> Result<()> {
        let res = unsafe {
            board_controller::release_session(
                self.board_id as c_int,
                self.json_brainflow_input_params.as_ptr(),
            )
        };
        Ok(check_brainflow_exit_code(res)?)
    }

    /// Get num of elements in ringbuffer.
    pub fn get_board_data_count(&self, preset: BrainFlowPresets) -> Result<usize> {
        let mut data_count = 0;
        let res = unsafe {
            board_controller::get_board_data_count(
                preset as c_int,
                &mut data_count,
                self.board_id as c_int,
                self.json_brainflow_input_params.as_ptr(),
            )
        };
        check_brainflow_exit_code(res)?;
        Ok(data_count as usize)
    }

    /// Get board data and remove data from ringbuffer
    pub fn get_board_data(&self, n_data_points: Option<usize>, preset: BrainFlowPresets) -> Result<Array2<f64>> {
        let num_rows = get_num_rows(self.board_id, preset)?;
        let num_samples = if let Some(n) = n_data_points {
            self.get_board_data_count(preset)?.min(n)
        } else {
            self.get_board_data_count(preset)?
        };

        let capacity = num_samples * num_rows;
        let mut data_buf = Vec::with_capacity(capacity);
        let res = unsafe {
            board_controller::get_board_data(
                num_samples as c_int,
                preset as c_int,
                data_buf.as_mut_ptr(),
                self.board_id as c_int,
                self.json_brainflow_input_params.as_ptr(),
            )
        };
        check_brainflow_exit_code(res)?;

        unsafe { data_buf.set_len(capacity) };

        Ok(Array2::from_shape_vec((num_rows, num_samples), data_buf)?)
    }

    /// Get specified amount of data or less if there is not enough data, doesnt remove data from ringbuffer.
    pub fn get_current_board_data(&self, num_samples: usize, preset: BrainFlowPresets) -> Result<Array2<f64>> {
        let num_rows = get_num_rows(self.board_id, preset)?;
        let capacity = num_samples * num_rows;
        let mut len = 0;
        let mut data_buf = Vec::with_capacity(capacity);
        let res = unsafe {
            board_controller::get_current_board_data(
                num_samples as c_int,
                preset as c_int,
                data_buf.as_mut_ptr(),
                &mut len,
                self.board_id as c_int,
                self.json_brainflow_input_params.as_ptr(),
            )
        };
        check_brainflow_exit_code(res)?;

        unsafe { data_buf.set_len(len as usize * num_rows) };
        Ok(Array2::from_shape_vec((num_rows, len as usize), data_buf)?)
    }

    /// Use this method carefully and only if you understand what you are doing, do NOT use it to start or stop streaming
    pub fn config_board<S: AsRef<str>>(&self, config: S) -> Result<String> {
        let config = CString::new(config.as_ref())?;
        let mut response_len = 0;
        let mut result_char_buffer: [c_char; 8192] = [0; 8192];

        let config = config.into_raw();
        let (res, response) = unsafe {
            let res = board_controller::config_board(
                config,
                result_char_buffer.as_mut_ptr(),
                &mut response_len,
                self.board_id as c_int,
                self.json_brainflow_input_params.as_ptr(),
            );
            let _ = CString::from_raw(config);
            let resp = CStr::from_ptr(result_char_buffer.as_ptr());

            (res, resp)
        };
        check_brainflow_exit_code(res)?;
        Ok(response
            .to_str()?
            .to_string())
    }

    /// Use this method carefully and only if you understand what you are doing.
    pub fn config_board_with_bytes(&self, bytes: Vec<i8>) -> Result<()> {
        let res = unsafe {
            board_controller::config_board_with_bytes(
                bytes.as_ptr(),
                bytes.len() as c_int,
                self.board_id as c_int,
                self.json_brainflow_input_params.as_ptr(),
            )
        };
        Ok(check_brainflow_exit_code(res)?)
    }

    /// Insert Marker to Data Stream.
    pub fn insert_marker(&self, value: f64, preset: BrainFlowPresets) -> Result<()> {
        let res = unsafe {
            board_controller::insert_marker(
                value as c_double,
                preset as c_int,
                self.board_id as c_int,
                self.json_brainflow_input_params.as_ptr(),
            )
        };
        Ok(check_brainflow_exit_code(res)?)
    }

    /// Get's the actual board id, can be different than provided.
    pub fn get_board_id(&self) -> BoardIds {
        return self.master_board_id;
    }
}

/// Set BrainFlow log level, use it only if you want to write your own messages to BrainFlow logger,
/// otherwise use [enable_board_logger], [enable_dev_board_logger] or [disable_board_logger].
pub fn set_log_level(log_level: LogLevels) -> Result<()> {
    let res = unsafe { board_controller::set_log_level_board_controller(log_level as c_int) };
    Ok(check_brainflow_exit_code(res)?)
}

/// Enable BrainFlow board logger with level INFO, uses stderr for log messages by default.
pub fn enable_board_logger() -> Result<()> {
    set_log_level(LogLevels::LevelInfo)
}

/// Disable BrainFlow board logger.
pub fn disable_board_logger() -> Result<()> {
    set_log_level(LogLevels::LevelOff)
}

/// Enable BrainFlow board logger with level TRACE, uses stderr for log messages by default.
pub fn enable_dev_board_logger() -> Result<()> {
    set_log_level(LogLevels::LevelTrace)
}

/// Redirect board logger from stderr to file, can be called any time.
pub fn set_log_file<S: AsRef<str>>(log_file: S) -> Result<()> {
    let log_file = log_file.as_ref();
    let log_file = CString::new(log_file)?;
    let res = unsafe { board_controller::set_log_file_board_controller(log_file.as_ptr()) };
    Ok(check_brainflow_exit_code(res)?)
}

/// Release all sessions
pub fn release_all_sessions() -> Result<()> {
    let res = unsafe {
        board_controller::release_all_sessions()
    };
    Ok(check_brainflow_exit_code(res)?)
}

macro_rules! gen_fn {
    ($fn_name:ident, $return_type:ident, $initial_value:literal, $doc:literal) => {
        paste! {
            #[doc = $doc]
            pub fn [<get_$fn_name>](board_id: BoardIds, preset: BrainFlowPresets) -> Result<$return_type> {
                let mut value = $initial_value;
                let res = unsafe { board_controller::[<get_$fn_name>](board_id as c_int, preset as c_int, &mut value) };
                check_brainflow_exit_code(res)?;
                Ok(value as $return_type)
            }
        }
    };
}

gen_fn!(sampling_rate, usize, 0, "Get sampling rate for a board.");
gen_fn!(
    package_num_channel,
    usize,
    0,
    "Get package num channel for a board."
);
gen_fn!(
    timestamp_channel,
    usize,
    0,
    "Get timestamp channel in resulting data table for a board."
);
gen_fn!(
    marker_channel,
    usize,
    0,
    "Get marker channel in resulting data table for a board."
);
gen_fn!(
    battery_channel,
    usize,
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
        let res = board_controller::log_message_board_controller(log_level as c_int, message);
        let _ = CString::from_raw(message);
        res
    };
    Ok(check_brainflow_exit_code(res)?)
}

/// Get board description as json.
pub fn get_board_descr(board_id: BoardIds, preset: BrainFlowPresets) -> Result<String> {
    const MAX_CHARS: usize = 16000;
    let mut response_len = 0;
    let mut result_char_buffer: [c_char; MAX_CHARS] = [0; MAX_CHARS];
    let (res, response) = unsafe {
        let res = board_controller::get_board_descr(board_id as c_int, preset as c_int, result_char_buffer.as_mut_ptr(), &mut response_len);
        let response = CStr::from_ptr(result_char_buffer.as_ptr());
        (res, response)
    };
    check_brainflow_exit_code(res)?;
    Ok(response.to_str()?.to_string())
}

/// Get names of EEG channels in 10-20 system if their location is fixed.
pub fn get_eeg_names(board_id: BoardIds, preset: BrainFlowPresets) -> Result<Vec<String>> {
    const MAX_CHARS: usize = 16000;
    let mut response_len = 0;
    let mut result_char_buffer: [c_char; MAX_CHARS] = [0; MAX_CHARS];
    let (res, response) = unsafe {
        let res = board_controller::get_eeg_names(board_id as c_int, preset as c_int, result_char_buffer.as_mut_ptr(), &mut response_len);
        let response = CStr::from_ptr(result_char_buffer.as_ptr());
        (res, response)
    };
    check_brainflow_exit_code(res)?;
    let names = response.to_str()?;

    Ok(names
        .split(',')
        .map(|s| s.to_string())
        .collect::<Vec<String>>())
}

/// Get presets for this board.
pub fn get_board_presets(board_id: BoardIds) -> Result<Vec<usize>> {
    let mut presets: Vec<c_int> = Vec::with_capacity(MAX_CHANNELS);
    let mut len = 0;
    let res = unsafe {
        board_controller::get_board_presets(
            board_id as c_int,
            presets.as_mut_ptr(),
            &mut len,
        )
    };
    check_brainflow_exit_code(res)?;
    unsafe { presets.set_len(len as usize) };
    let presets_casted = presets.into_iter().map(|c| c as usize).collect::<Vec<usize>>();
    Ok(presets_casted)
}

/// Get BoardShim version.
pub fn get_version() -> Result<String> {
    const MAX_CHARS: usize = 64;
    let mut response_len = 0;
    let mut result_char_buffer: [c_char; MAX_CHARS] = [0; MAX_CHARS];

    let (res, response) = unsafe {
        let res = board_controller::get_version_board_controller(result_char_buffer.as_mut_ptr(), &mut response_len, MAX_CHARS as i32);
        let response = CStr::from_ptr(result_char_buffer.as_ptr());
        (res, response)
    };
    check_brainflow_exit_code(res)?;
    let version = response.to_str()?;

    Ok(version.to_string())
}

/// Get device name.
pub fn get_device_name(board_id: BoardIds, preset: BrainFlowPresets) -> Result<String> {
    const MAX_CHARS: usize = 4096;
    let mut response_len = 0;
    let mut result_char_buffer: [c_char; MAX_CHARS] = [0; MAX_CHARS];

    let (res, response) = unsafe {
        let res = board_controller::get_device_name(board_id as c_int, preset as c_int, result_char_buffer.as_mut_ptr(), &mut response_len);
        let response = CStr::from_ptr(result_char_buffer.as_ptr());
        (res, response)
    };
    check_brainflow_exit_code(res)?;
    Ok(response.to_str()?.to_string())
}

macro_rules! gen_vec_fn {
    ($fn_name:ident, $doc:literal) => {
        paste! {
            #[doc = $doc]
            pub fn [<get_$fn_name>](board_id: BoardIds, preset: BrainFlowPresets) -> Result<Vec<usize>> {
                let mut channels: Vec<c_int> = Vec::with_capacity(MAX_CHANNELS);
                let mut len = 0;
                let res = unsafe {
                    board_controller::[<get_$fn_name>](
                        board_id as c_int,
                        preset as c_int,
                        channels.as_mut_ptr(),
                        &mut len,
                    )
                };
                check_brainflow_exit_code(res)?;
                unsafe { channels.set_len(len as usize) };
                let channels_casted = channels.into_iter().map(|c| c as usize).collect::<Vec<usize>>();
                Ok(channels_casted)
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
    rotation_channels,
    "Get list of rotation channels in resulting data table for a board."
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
gen_vec_fn!(
    magnetometer_channels,
    "Get list of magnetometer channels in resulting data table for a board."
);

#[cfg(test)]
mod tests {
    #[cfg(test)]
    mod functions {
        use crate::test_helpers::assertions::assert_regex_matches;
        use crate::test_helpers::consts::VERSION_PATTERN;
        use crate::board_shim::{get_version, get_device_name, get_eeg_names, get_board_descr};
        use crate::{BoardIds, BrainFlowPresets};

        #[test]
        fn test_it_gets_the_version() {
            assert_regex_matches(VERSION_PATTERN, get_version().unwrap().as_str());
        }

        #[test]
        fn test_get_device_name() {
            assert_eq!("Synthetic", get_device_name(BoardIds::SyntheticBoard, BrainFlowPresets::DefaultPreset).unwrap());
        }

        #[test]
        fn test_get_eeg_names() {
            assert_eq!(vec!["Fz", "C3", "Cz", "C4", "Pz", "PO7", "Oz", "PO8", "F5", "F7", "F3", "F1", "F2", "F4", "F6", "F8"],
                       get_eeg_names(BoardIds::SyntheticBoard, BrainFlowPresets::DefaultPreset).unwrap());

        }

        #[test]
        fn test_get_board_descr() {
            let pattern = r#"^.*"name"\w*:\w*"Synthetic".*$"#;

            assert_regex_matches(pattern,
                       get_board_descr(BoardIds::SyntheticBoard, BrainFlowPresets::DefaultPreset).unwrap().as_str());
        }
    }


    #[cfg(test)]
    mod board_shim {
        use crate::board_shim::BoardShim;
        use crate::BoardIds;
        use crate::brainflow_input_params::BrainFlowInputParams;

        fn board_shim() -> BoardShim {
            BoardShim::new(BoardIds::SyntheticBoard,
                           BrainFlowInputParams::default()).unwrap()
        }

        #[test]
        fn test_config_board() {
            let board = board_shim();
            board.prepare_session();
            // synthetic board doesnt send any message back as it ignores config calls
            assert_eq!("", board.config_board("x123456X").unwrap());

        }
    }

}