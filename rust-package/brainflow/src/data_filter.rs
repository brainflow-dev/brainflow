use std::os::raw::c_int;
use std::{ffi::CString, os::raw::c_double};

use ffi::{AggOperations, FilterTypes, LogLevels, NoiseTypes};

use crate::error::{BrainFlowError, Error};
use crate::{check_brainflow_exit_code, ffi, Result};

pub fn set_log_level(log_level: LogLevels) -> Result<()> {
    let res = unsafe { ffi::set_log_level(log_level as c_int) };
    Ok(check_brainflow_exit_code(res)?)
}

pub fn enable_data_logger() -> Result<()> {
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

pub fn perform_lowpass(
    mut data: Vec<f64>,
    sampling_rate: usize,
    cutoff: f64,
    order: usize,
    filter_type: FilterTypes,
    ripple: f64,
) -> Result<()> {
    let res = unsafe {
        ffi::perform_lowpass(
            data.as_mut_ptr() as *mut c_double,
            data.len() as c_int,
            sampling_rate as c_int,
            cutoff as c_double,
            order as c_int,
            filter_type as c_int,
            ripple as c_double,
        )
    };
    Ok(check_brainflow_exit_code(res)?)
}

pub fn perform_highpass(
    mut data: Vec<f64>,
    sampling_rate: usize,
    cutoff: f64,
    order: usize,
    filter_type: FilterTypes,
    ripple: f64,
) -> Result<()> {
    let res = unsafe {
        ffi::perform_highpass(
            data.as_mut_ptr() as *mut c_double,
            data.len() as c_int,
            sampling_rate as c_int,
            cutoff as c_double,
            order as c_int,
            filter_type as c_int,
            ripple as c_double,
        )
    };
    Ok(check_brainflow_exit_code(res)?)
}

pub fn perform_bandpass(
    mut data: Vec<f64>,
    sampling_rate: usize,
    center_freq: f64,
    band_width: f64,
    order: usize,
    filter_type: FilterTypes,
    ripple: f64,
) -> Result<()> {
    let res = unsafe {
        ffi::perform_bandpass(
            data.as_mut_ptr() as *mut c_double,
            data.len() as c_int,
            sampling_rate as c_int,
            center_freq as c_double,
            band_width as c_double,
            order as c_int,
            filter_type as c_int,
            ripple as c_double,
        )
    };
    Ok(check_brainflow_exit_code(res)?)
}

pub fn perform_bandstop(
    mut data: Vec<f64>,
    sampling_rate: usize,
    center_freq: f64,
    band_width: f64,
    order: usize,
    filter_type: FilterTypes,
    ripple: f64,
) -> Result<()> {
    let res = unsafe {
        ffi::perform_bandstop(
            data.as_mut_ptr() as *mut c_double,
            data.len() as c_int,
            sampling_rate as c_int,
            center_freq as c_double,
            band_width as c_double,
            order as c_int,
            filter_type as c_int,
            ripple as c_double,
        )
    };
    Ok(check_brainflow_exit_code(res)?)
}

pub fn remove_environmental_noise(
    mut data: Vec<f64>,
    sampling_rate: usize,
    noise_type: NoiseTypes,
) -> Result<()> {
    let res = unsafe {
        ffi::remove_environmental_noise(
            data.as_mut_ptr() as *mut c_double,
            data.len() as c_int,
            sampling_rate as c_int,
            noise_type as c_int,
        )
    };
    Ok(check_brainflow_exit_code(res)?)
}

pub fn perform_rolling_filter(
    mut data: Vec<f64>,
    period: usize,
    agg_operation: AggOperations,
) -> Result<()> {
    let res = unsafe {
        ffi::perform_rolling_filter(
            data.as_mut_ptr() as *mut c_double,
            data.len() as c_int,
            period as c_int,
            agg_operation as c_int,
        )
    };
    Ok(check_brainflow_exit_code(res)?)
}

pub fn perform_downsampling(
    mut data: Vec<f64>,
    period: usize,
    agg_operation: AggOperations,
) -> Result<Vec<f64>> {
    if period == 0 {
        return Err(Error::BrainFlowError(BrainFlowError::InvalidArgumentsError));
    }
    let mut output = Vec::<f64>::with_capacity(data.len() / period as usize);
    let res = unsafe {
        ffi::perform_downsampling(
            data.as_mut_ptr() as *mut c_double,
            data.len() as c_int,
            period as c_int,
            agg_operation as c_int,
            output.as_mut_ptr() as *mut c_double,
        )
    };
    check_brainflow_exit_code(res)?;
    Ok(output)
}

pub fn perform_wavelet_transform<S: AsRef<str>>(
    mut data: Vec<f64>,
    wavelet: S,
    decomposition_level: usize,
) -> Result<Vec<f64>> {
    let mut output = Vec::<f64>::with_capacity(data.len() + 2 * decomposition_level * (40 + 1));
    let mut decomposition_lengths = Vec::<usize>::with_capacity(decomposition_level + 1);
    let wavelet = CString::new(wavelet.as_ref())?;
    let res = unsafe {
        ffi::perform_wavelet_transform(
            data.as_mut_ptr() as *mut c_double,
            data.len() as c_int,
            wavelet.as_ptr(),
            decomposition_level as c_int,
            output.as_mut_ptr() as *mut c_double,
            decomposition_lengths.as_mut_ptr() as *mut c_int,
        )
    };
    check_brainflow_exit_code(res)?;
    Ok(output)
}
