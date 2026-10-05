use getset::Getters;
use ndarray::{Array1, Array2, Array3, ArrayBase};
use num::Complex;
use num_complex::Complex64;
use std::os::raw::c_int;
use std::{ffi::CString, ffi::CStr, os::raw::c_double};
use std::os::raw::c_char;
use std::convert::TryFrom;

use crate::error::{BrainFlowError, Error};
use crate::ffi::data_handler;
use crate::{
    check_brainflow_exit_code, AggOperations, DetrendOperations, FilterTypes, LogLevels,
    NoiseTypes, Result, WindowOperations, WaveletTypes, WaveletExtensionTypes, WaveletDenoisingTypes, ThresholdTypes, NoiseEstimationLevelTypes,
};

fn invalid_arguments() -> Error {
    Error::BrainFlowError(BrainFlowError::InvalidArgumentsError)
}

fn native_int(value: usize) -> Result<c_int> {
    c_int::try_from(value).map_err(|_| invalid_arguments())
}

fn checked_product(left: usize, right: usize) -> Result<usize> {
    let value = left.checked_mul(right).ok_or_else(invalid_arguments)?;
    native_int(value)?;
    Ok(value)
}

fn selected_data(data: &Array2<f64>, channels: &[usize]) -> Result<Vec<f64>> {
    if channels.is_empty() || data.ncols() == 0 || channels.iter().any(|&c| c >= data.nrows()) {
        return Err(invalid_arguments());
    }
    let mut selected = Vec::with_capacity(checked_product(channels.len(), data.ncols())?);
    // Preserve the requested order and duplicate channels consistently with other bindings.
    for &channel in channels {
        selected.extend(data.row(channel).iter().copied());
    }
    Ok(selected)
}


/// Set BrainFlow data logger log level.
/// Use it only if you want to write your own messages to BrainFlow logger.
/// Otherwise use [enable_data_logger], [enable_dev_data_logger] or [disable_data_logger].
pub fn set_log_level(log_level: LogLevels) -> Result<()> {
    let res = unsafe { data_handler::set_log_level_data_handler(log_level as c_int) };
    Ok(check_brainflow_exit_code(res)?)
}

/// Enable data logger with level INFO, uses stderr for log messages by default
pub fn enable_data_logger() -> Result<()> {
    set_log_level(LogLevels::LevelInfo)
}

/// Disable data logger.
pub fn disable_data_logger() -> Result<()> {
    set_log_level(LogLevels::LevelOff)
}

/// Enable data logger with level TRACE, uses stderr for log messages by default.
pub fn enable_dev_data_logger() -> Result<()> {
    set_log_level(LogLevels::LevelTrace)
}

/// Write your own log message to BrainFlow board logger, use it if you wanna have single logger for your own code and BrainFlow's code.
pub fn log_message<S: AsRef<str>>(log_level: LogLevels, message: S) -> Result<()> {
    let message = message.as_ref();
    let message = CString::new(message)?.into_raw();
    let res = unsafe {
        let res = data_handler::log_message_data_handler(log_level as c_int, message);
        let _ = CString::from_raw(message);
        res
    };
    Ok(check_brainflow_exit_code(res)?)
}

/// Redirect data logger from stderr to file, can be called any time.
pub fn set_log_file<S: AsRef<str>>(log_file: S) -> Result<()> {
    let log_file = log_file.as_ref();
    let log_file = CString::new(log_file)?;
    let res = unsafe { data_handler::set_log_file_data_handler(log_file.as_ptr()) };
    Ok(check_brainflow_exit_code(res)?)
}

/// Apply low pass filter to provided data.
pub fn perform_lowpass(
    data: &mut [f64],
    sampling_rate: usize,
    cutoff: f64,
    order: usize,
    filter_type: FilterTypes,
    ripple: f64,
) -> Result<()> {
    let res = unsafe {
        data_handler::perform_lowpass(
            data.as_mut_ptr() as *mut c_double,
            native_int(data.len())?,
            native_int(sampling_rate)?,
            cutoff as c_double,
            native_int(order)?,
            filter_type as c_int,
            ripple as c_double,
        )
    };
    check_brainflow_exit_code(res)?;
    Ok(())
}

/// Apply high pass filter to provided data.
pub fn perform_highpass(
    data: &mut [f64],
    sampling_rate: usize,
    cutoff: f64,
    order: usize,
    filter_type: FilterTypes,
    ripple: f64,
) -> Result<()> {
    let res = unsafe {
        data_handler::perform_highpass(
            data.as_mut_ptr() as *mut c_double,
            native_int(data.len())?,
            native_int(sampling_rate)?,
            cutoff as c_double,
            native_int(order)?,
            filter_type as c_int,
            ripple as c_double,
        )
    };
    check_brainflow_exit_code(res)?;
    Ok(())
}

/// Apply band pass filter to provided data.
pub fn perform_bandpass(
    data: &mut [f64],
    sampling_rate: usize,
    start_freq: f64,
    stop_freq: f64,
    order: usize,
    filter_type: FilterTypes,
    ripple: f64,
) -> Result<()> {
    let res = unsafe {
        data_handler::perform_bandpass(
            data.as_mut_ptr() as *mut c_double,
            native_int(data.len())?,
            native_int(sampling_rate)?,
            start_freq as c_double,
            stop_freq as c_double,
            native_int(order)?,
            filter_type as c_int,
            ripple as c_double,
        )
    };
    check_brainflow_exit_code(res)?;
    Ok(())
}

/// Apply band stop filter to provided data.
pub fn perform_bandstop(
    data: &mut [f64],
    sampling_rate: usize,
    start_freq: f64,
    stop_freq: f64,
    order: usize,
    filter_type: FilterTypes,
    ripple: f64,
) -> Result<()> {
    let res = unsafe {
        data_handler::perform_bandstop(
            data.as_mut_ptr() as *mut c_double,
            native_int(data.len())?,
            native_int(sampling_rate)?,
            start_freq as c_double,
            stop_freq as c_double,
            native_int(order)?,
            filter_type as c_int,
            ripple as c_double,
        )
    };
    check_brainflow_exit_code(res)?;
    Ok(())
}

/// Remove environmantal noise using notch filter.
pub fn remove_environmental_noise(
    data: &mut [f64],
    sampling_rate: usize,
    noise_type: NoiseTypes,
) -> Result<()> {
    let res = unsafe {
        data_handler::remove_environmental_noise(
            data.as_mut_ptr() as *mut c_double,
            native_int(data.len())?,
            native_int(sampling_rate)?,
            noise_type as c_int,
        )
    };
    check_brainflow_exit_code(res)?;
    Ok(())
}

/// Smooth data using moving average or median.
pub fn perform_rolling_filter(
    data: &mut [f64],
    period: usize,
    agg_operation: AggOperations,
) -> Result<()> {
    let res = unsafe {
        data_handler::perform_rolling_filter(
            data.as_mut_ptr() as *mut c_double,
            native_int(data.len())?,
            native_int(period)?,
            agg_operation as c_int,
        )
    };
    check_brainflow_exit_code(res)?;
    Ok(())
}

/// Calc stddev.
pub fn calc_stddev(
    data: &mut [f64],
    start_pos: usize,
    end_pos: usize
) -> Result<f64> {
    if start_pos >= end_pos || end_pos > data.len() {
        return Err(invalid_arguments());
    }
    let mut output = 0.0 as f64;
    let res = unsafe {
        data_handler::calc_stddev(
            data.as_mut_ptr() as *mut c_double,
            native_int(start_pos)?,
            native_int(end_pos)?,
            &mut output,
        )
    };
    check_brainflow_exit_code(res)?;
    Ok(output as f64)
}

/// Get Railed Percentage.
pub fn get_railed_percentage(
    data: &mut [f64],
    data_size: usize,
    gain: usize
) -> Result<f64> {
    if data_size == 0 || data_size > data.len() {
        return Err(invalid_arguments());
    }
    let mut output = 0.0 as f64;
    let res = unsafe {
        data_handler::get_railed_percentage(
            data.as_mut_ptr() as *mut c_double,
            native_int(data_size)?,
            native_int(gain)?,
            &mut output,
        )
    };
    check_brainflow_exit_code(res)?;
    Ok(output as f64)
}

/// Calculate oxygen level.
pub fn get_oxygen_level(
    ppg_ir: &mut [f64],
    ppg_red: &mut [f64],
    sampling_rate: usize,
    coef1: f64,
    coef2: f64,
    coef3: f64,
) -> Result<f64> {
    if ppg_ir.len() != ppg_red.len() {
        return Err(invalid_arguments());
    }
    let mut output = 0.0 as f64;
    let res = unsafe {
        data_handler::get_oxygen_level(
            ppg_ir.as_mut_ptr() as *mut c_double,
            ppg_red.as_mut_ptr() as *mut c_double,
            native_int(ppg_red.len())?,
            native_int(sampling_rate)?,
            coef1 as c_double,
            coef2 as c_double,
            coef3 as c_double,
            &mut output,
        )
    };
    check_brainflow_exit_code(res)?;
    Ok(output as f64)
}

/// Calculate heart rate.
pub fn get_heart_rate(
    ppg_ir: &mut [f64],
    ppg_red: &mut [f64],
    sampling_rate: usize,
    fft_size: usize,
) -> Result<f64> {
    if ppg_ir.len() != ppg_red.len() {
        return Err(invalid_arguments());
    }
    let mut output = 0.0 as f64;
    let res = unsafe {
        data_handler::get_heart_rate(
            ppg_ir.as_mut_ptr() as *mut c_double,
            ppg_red.as_mut_ptr() as *mut c_double,
            native_int(ppg_red.len())?,
            native_int(sampling_rate)?,
            native_int(fft_size)?,
            &mut output,
        )
    };
    check_brainflow_exit_code(res)?;
    Ok(output as f64)
}

/// Perform data downsampling, it doesnt apply lowpass filter for you, it just aggregates several data points.
pub fn perform_downsampling(
    data: &mut [f64],
    period: usize,
    agg_operation: AggOperations,
) -> Result<Vec<f64>> {
    if period == 0 {
        return Err(Error::BrainFlowError(BrainFlowError::InvalidArgumentsError));
    }
    let output_len = data.len() / period as usize;
    let mut output = Vec::<f64>::with_capacity(output_len);
    let res = unsafe {
        data_handler::perform_downsampling(
            data.as_mut_ptr() as *mut c_double,
            native_int(data.len())?,
            native_int(period)?,
            agg_operation as c_int,
            output.as_mut_ptr() as *mut c_double,
        )
    };
    check_brainflow_exit_code(res)?;
    unsafe { output.set_len(output_len) }
    Ok(output)
}

/// Data struct for output of wavelet transformations.
#[derive(Getters, Clone)]
#[getset(get = "pub")]
pub struct WaveletTransform {
    coefficients: Vec<f64>,
    decomposition_level: usize,
    decomposition_lengths: Vec<usize>,
    wavelet: WaveletTypes,
    extension: WaveletExtensionTypes,
    original_data_len: usize,
}
impl WaveletTransform {
    pub fn new(
        capacity: usize,
        decomposition_level: usize,
        wavelet: WaveletTypes,
        extension: WaveletExtensionTypes,
        original_data_len: usize,
    ) -> Self {
        Self {
            coefficients: Vec::with_capacity(capacity),
            decomposition_level,
            decomposition_lengths: Vec::with_capacity(decomposition_level + 1),
            wavelet,
            extension,
            original_data_len,
        }
    }

    /// Create new WaveletTransform with coefficients.
    /// This function can be used to create input data for [perform_inverse_wavelet_transform].
    pub fn with_coefficients(
        coefficients: Vec<f64>,
        decomposition_level: usize,
        decomposition_lengths: Vec<usize>,
        wavelet: WaveletTypes,
        extension: WaveletExtensionTypes,
        original_data_len: usize,
        
    ) -> Self {
        Self {
            coefficients,
            decomposition_level,
            decomposition_lengths,
            wavelet,
            extension,
            original_data_len,
        }
    }
}

/// Perform wavelet transform.
pub fn perform_wavelet_transform(
    data: &mut [f64],
    wavelet: WaveletTypes,
    decomposition_level: usize,
    extension: WaveletExtensionTypes,
) -> Result<WaveletTransform> {
    if decomposition_level == 0 || decomposition_level > 100 || data.is_empty() {
        return Err(invalid_arguments());
    }
    native_int(data.len())?;
    let capacity = data.len().checked_add(checked_product(decomposition_level, 82)?)
        .ok_or_else(invalid_arguments)?;
    native_int(capacity)?;
    let mut wavelet_transform = WaveletTransform::new(
        capacity,
        decomposition_level,
        wavelet,
        extension,
        data.len(),
    );
    let mut lengths = vec![0 as c_int; decomposition_level + 1];
    let res = unsafe {
        let output = wavelet_transform.coefficients.as_mut_ptr() as *mut c_double;
        let decomposition_lengths = lengths.as_mut_ptr();
        data_handler::perform_wavelet_transform(
            data.as_mut_ptr() as *mut c_double,
            native_int(data.len())?,
            wavelet as c_int,
            native_int(decomposition_level)?,
            extension as c_int,
            output,
            decomposition_lengths,
        )
    };
    check_brainflow_exit_code(res)?;
    let mut total = 0usize;
    for &length in &lengths {
        if length <= 0 {
            return Err(invalid_arguments());
        }
        total = total.checked_add(length as usize).ok_or_else(invalid_arguments)?;
    }
    if total > capacity {
        return Err(invalid_arguments());
    }
    unsafe { wavelet_transform.coefficients.set_len(total); }
    wavelet_transform.decomposition_lengths = lengths.into_iter().map(|v| v as usize).collect();
    Ok(wavelet_transform)
}


/// Restore data from a single detailed coef.
pub fn restore_data_from_wavelet_detailed_coeffs(
    data: &mut [f64],
    wavelet: WaveletTypes,
    decomposition_level: usize,
    level_to_restore: usize,
) -> Result<Vec<f64>> {
    let output_len = data.len();
    let mut output = Vec::<f64>::with_capacity(output_len);
    let res = unsafe {
        data_handler::restore_data_from_wavelet_detailed_coeffs(
            data.as_mut_ptr() as *mut c_double,
            native_int(data.len())?,
            wavelet as c_int,
            native_int(decomposition_level)?,
            native_int(level_to_restore)?,
            output.as_mut_ptr() as *mut c_double,
        )
    };
    check_brainflow_exit_code(res)?;
    unsafe { output.set_len(output_len) }
    Ok(output)
}

/// Detect Peaks using z score method.
pub fn detect_peaks_z_score(
    data: &mut [f64],
    lag: usize,
    threshold: f64,
    influence: f64,
) -> Result<Vec<f64>> {
    let output_len = data.len();
    let mut output = Vec::<f64>::with_capacity(output_len);
    let res = unsafe {
        data_handler::detect_peaks_z_score(
            data.as_mut_ptr() as *mut c_double,
            native_int(data.len())?,
            native_int(lag)?,
            threshold as c_double,
            influence as c_double,
            output.as_mut_ptr() as *mut c_double,
        )
    };
    check_brainflow_exit_code(res)?;
    unsafe { output.set_len(output_len) }
    Ok(output)
}

/// Perform inverse wavelet transform.
pub fn perform_inverse_wavelet_transform(wavelet_transform: WaveletTransform) -> Result<Vec<f64>> {
    let mut wavelet_transform = wavelet_transform;
    let level = wavelet_transform.decomposition_level;
    if level == 0 || level > 100 || wavelet_transform.decomposition_lengths.len() != level + 1
        || wavelet_transform.original_data_len == 0
        || wavelet_transform.original_data_len > wavelet_transform.coefficients.len() {
        return Err(invalid_arguments());
    }
    let mut lengths = Vec::with_capacity(level + 1);
    let mut total = 0usize;
    for &length in &wavelet_transform.decomposition_lengths {
        if length == 0 { return Err(invalid_arguments()); }
        lengths.push(native_int(length)?);
        total = total.checked_add(length).ok_or_else(invalid_arguments)?;
    }
    if total != wavelet_transform.coefficients.len() {
        return Err(invalid_arguments());
    }
    native_int(wavelet_transform.original_data_len)?;
    let mut output = Vec::<f64>::with_capacity(wavelet_transform.original_data_len);
    let res = unsafe {
        data_handler::perform_inverse_wavelet_transform_checked(
            wavelet_transform.coefficients.as_mut_ptr() as *mut c_double,
            native_int(wavelet_transform.coefficients.len())?,
            native_int(wavelet_transform.original_data_len)?,
            wavelet_transform.wavelet as c_int,
            native_int(wavelet_transform.decomposition_level)?,
            wavelet_transform.extension as c_int,
            lengths.as_mut_ptr(),
            native_int(lengths.len())?,
            output.as_mut_ptr() as *mut c_double,
            native_int(wavelet_transform.original_data_len)?,
        )
    };
    check_brainflow_exit_code(res)?;
    unsafe { output.set_len(wavelet_transform.original_data_len) }
    Ok(output)
}

/// Perform wavelet denoising.
pub fn perform_wavelet_denoising(
    data: &mut [f64],
    wavelet: WaveletTypes,
    decomposition_level: usize,
    wavelet_denoising: WaveletDenoisingTypes,
    wavelet_threshold: ThresholdTypes,
    extension: WaveletExtensionTypes,
    noise_level: NoiseEstimationLevelTypes,
) -> Result<()> {
    let res = unsafe {
        data_handler::perform_wavelet_denoising(
            data.as_mut_ptr() as *mut c_double,
            native_int(data.len())?,
            wavelet as c_int,
            native_int(decomposition_level)?,
            wavelet_denoising as c_int,
            wavelet_threshold as c_int,
            extension as c_int,
            noise_level as c_int,
        )
    };
    check_brainflow_exit_code(res)?;
    Ok(())
}

/// Calculate filters and the corresponding eigenvalues using the Common Spatial Patterns.
pub fn get_csp(
    data: &Array3<f64>,
    labels: &Array1<f64>,
) -> Result<(Array2<f64>, Array1<f64>)> {
    let shape = data.shape();
    let n_epochs = shape[0];
    let n_channels = shape[1];
    let n_times = shape[2];
    if labels.len() != n_epochs || n_epochs == 0 || n_channels == 0 || n_times == 0 {
        return Err(invalid_arguments());
    }
    checked_product(n_channels, n_channels)?;
    native_int(data.len())?;
    let data: Vec<f64> = data.into_iter().cloned().collect();

    let labels: Vec<f64> = labels.into_iter().cloned().collect();

    let mut output_filters = Vec::<f64>::with_capacity(n_channels * n_channels);
    let mut output_eigenvalues = Vec::<f64>::with_capacity(n_channels);

    let res = unsafe {
        data_handler::get_csp(
            data.as_ptr() as *const c_double,
            labels.as_ptr() as *const c_double,
            native_int(n_epochs)?,
            native_int(n_channels)?,
            native_int(n_times)?,
            output_filters.as_mut_ptr() as *mut c_double,
            output_eigenvalues.as_mut_ptr() as *mut c_double,
        )
    };
    check_brainflow_exit_code(res)?;

    unsafe { output_filters.set_len(n_channels * n_channels) };
    unsafe { output_eigenvalues.set_len(n_channels) };

    let output_filters = ArrayBase::from_vec(output_filters);
    let output_filters = output_filters.into_shape((n_channels, n_channels)).unwrap();
    let output_eigenvalues = Array1::from(output_eigenvalues);
    Ok((output_filters, output_eigenvalues))
}

/// Perform data windowing.
pub fn get_window(window_function: WindowOperations, window_len: usize) -> Result<Vec<f64>> {
    native_int(window_len)?;
    let mut output = Vec::<f64>::with_capacity(window_len);
    let res = unsafe {
        data_handler::get_window(
            window_function as c_int,
            native_int(window_len)?,
            output.as_mut_ptr() as *mut c_double,
        )
    };
    check_brainflow_exit_code(res)?;

    unsafe { output.set_len(window_len) };
    Ok(output)
}

/// Perform direct FFT.
pub fn perform_fft(data: &mut [f64], window_function: WindowOperations) -> Result<Vec<Complex64>> {
    let mut output_re = Vec::<f64>::with_capacity(data.len() / 2 + 1);
    let mut output_im = Vec::<f64>::with_capacity(data.len() / 2 + 1);
    let res = unsafe {
        data_handler::perform_fft(
            data.as_mut_ptr() as *mut c_double,
            native_int(data.len())?,
            window_function as c_int,
            output_re.as_mut_ptr() as *mut c_double,
            output_im.as_mut_ptr() as *mut c_double,
        )
    };
    check_brainflow_exit_code(res)?;

    unsafe { output_re.set_len(data.len() / 2 + 1) };
    unsafe { output_im.set_len(data.len() / 2 + 1) };
    let output = output_re
        .into_iter()
        .zip(output_im)
        .map(|(re, im)| Complex { re, im })
        .collect();
    Ok(output)
}

/// Perform inverse FFT.
pub fn perform_ifft(data: &[Complex64], original_data_len: usize) -> Result<Vec<f64>> {
    native_int(original_data_len)?;
    if data.len() < 2 || original_data_len != checked_product(data.len() - 1, 2)? {
        return Err(invalid_arguments());
    }
    let mut restored_data = Vec::<f64>::with_capacity(original_data_len);
    let (mut input_re, mut input_im): (Vec<f64>, Vec<f64>) =
        data.iter().map(|d| (d.re, d.im)).unzip();
    let res = unsafe {
        data_handler::perform_ifft(
            input_re.as_mut_ptr() as *mut c_double,
            input_im.as_mut_ptr() as *mut c_double,
            native_int(original_data_len)?,
            restored_data.as_mut_ptr() as *mut c_double,
        )
    };
    check_brainflow_exit_code(res)?;

    unsafe { restored_data.set_len(original_data_len) };
    Ok(restored_data)
}

/// Detrend data.
pub fn detrend(data: &mut [f64], detrend_operation: DetrendOperations) -> Result<()> {
    let res = unsafe {
        data_handler::detrend(
            data.as_mut_ptr() as *mut c_double,
            native_int(data.len())?,
            detrend_operation as c_int,
        )
    };
    Ok(check_brainflow_exit_code(res)?)
}

/// Data struct for output of PSD calculations.
#[derive(Getters, Clone)]
#[getset(get = "pub")]
pub struct Psd {
    amplitude: Vec<f64>,
    frequency: Vec<f64>,
}

/// Calculate PSD.
pub fn get_psd(
    data: &mut [f64],
    sampling_rate: usize,
    window_function: WindowOperations,
) -> Result<Psd> {
    let mut amplitude = Vec::<f64>::with_capacity(data.len() / 2 + 1);
    let mut frequency = Vec::<f64>::with_capacity(data.len() / 2 + 1);
    let res = unsafe {
        data_handler::get_psd(
            data.as_mut_ptr() as *mut c_double,
            native_int(data.len())?,
            native_int(sampling_rate)?,
            window_function as c_int,
            amplitude.as_mut_ptr() as *mut c_double,
            frequency.as_mut_ptr() as *mut c_double,
        )
    };
    check_brainflow_exit_code(res)?;

    unsafe { amplitude.set_len(data.len() / 2 + 1) };
    unsafe { frequency.set_len(data.len() / 2 + 1) };
    Ok(Psd {
        amplitude,
        frequency,
    })
}

/// Calculate PSD using Welch method.
pub fn get_psd_welch(
    data: &mut [f64],
    nfft: usize,
    overlap: usize,
    sampling_rate: usize,
    window_function: WindowOperations,
) -> Result<Psd> {
    native_int(nfft)?;
    if nfft == 0 || nfft > data.len() || nfft % 2 != 0 || overlap >= nfft {
        return Err(invalid_arguments());
    }
    let mut amplitude = Vec::<f64>::with_capacity(nfft / 2 + 1);
    let mut frequency = Vec::<f64>::with_capacity(nfft / 2 + 1);
    let res = unsafe {
        data_handler::get_psd_welch(
            data.as_mut_ptr() as *mut c_double,
            native_int(data.len())?,
            native_int(nfft)?,
            native_int(overlap)?,
            native_int(sampling_rate)?,
            window_function as c_int,
            amplitude.as_mut_ptr() as *mut c_double,
            frequency.as_mut_ptr() as *mut c_double,
        )
    };
    check_brainflow_exit_code(res)?;

    unsafe { amplitude.set_len(nfft / 2 + 1) };
    unsafe { frequency.set_len(nfft / 2 + 1) };
    Ok(Psd {
        amplitude,
        frequency,
    })
}

/// Data struct for exg bands
#[derive(Getters, Clone)]
#[getset(get = "pub")]
pub struct Band {
    pub freq_start: f64,
    pub freq_stop: f64,
}

/// Calculate ICA
pub fn perform_ica_select_channels(
    data: Array2<f64>,
    num_components: usize,
    channels: Vec<usize>
) -> Result<(Vec<f64>, Vec<f64>, Vec<f64>, Vec<f64>)> {
    let shape = data.shape();
    let (rows, cols) = (channels.len(), shape[1]);
    if num_components < 2 || num_components > rows || cols < 2 {
        return Err(invalid_arguments());
    }
    let mut raw_data = selected_data(&data, &channels)?;
    checked_product(num_components, num_components)?;
    checked_product(rows, num_components)?;
    checked_product(cols, num_components)?;

    let mut temp_w = Vec::with_capacity(num_components * num_components);
    let mut temp_k = Vec::with_capacity(rows * num_components);
    let mut temp_a = Vec::with_capacity(num_components * rows);
    let mut temp_s = Vec::with_capacity(cols * num_components);

    let res = unsafe {
        data_handler::perform_ica(
            raw_data.as_mut_ptr() as *mut c_double,
            native_int(rows)?,
            native_int(cols)?,
            native_int(num_components)?,
            temp_w.as_mut_ptr() as *mut c_double,
            temp_k.as_mut_ptr() as *mut c_double,
            temp_a.as_mut_ptr() as *mut c_double,
            temp_s.as_mut_ptr() as *mut c_double,
        )
    };
    check_brainflow_exit_code(res)?;
    unsafe {
        temp_w.set_len(num_components * num_components);
        temp_k.set_len(rows * num_components);
        temp_a.set_len(rows * num_components);
        temp_s.set_len(cols * num_components);
    }
    Ok((temp_w, temp_k, temp_a, temp_s))
}

/// Calculate ICA
pub fn perform_ica(
    data: Array2<f64>,
    num_components: usize
) -> Result<(Vec<f64>, Vec<f64>, Vec<f64>, Vec<f64>)> {
    let shape = data.shape();
    let channels = (0..shape[0]).collect();
    perform_ica_select_channels(data, num_components, channels)
}

/// Return normalized channel-mean band powers and coefficients of variation (population
/// stddev / mean of absolute channel powers). Zero-power bands have zero variation;
/// an all-zero total returns zero normalized powers.
/// Filtering removes DC and applies padded, initialized zero-phase 48-52 and 58-62 Hz
/// notches only when their upper edges are below 0.9 * Nyquist. Preprocessing is independent
/// of the requested output bands; no automatic passband is applied. Margins estimated from
/// the filter cascade's impulse tail are discarded at both ends; supply surrounding samples and account
/// for the resulting delay in live analysis. Without filtering there is no preprocessing
/// or trimming. At least max(8, 2 * get_nearest_power_of_two(sampling_rate)) samples must remain.
/// Data and edges must be finite. Bands require 0 <= start < stop <= Nyquist.
/// Mains notches also attenuate overlapping bands.
pub fn get_custom_band_powers(
    data: Array2<f64>,
    bands: Vec<Band>,
    eeg_channels: Vec<usize>,
    sampling_rate: usize,
    apply_filters: bool,
) -> Result<(Vec<f64>, Vec<f64>)> {
    let shape = data.shape();
    let (rows, cols) = (eeg_channels.len(), shape[1]);
    let mut raw_data = selected_data(&data, &eeg_channels)?;

    let (mut x, mut y): (Vec<_>, Vec<_>) = bands.into_iter().map(|Band{freq_start, freq_stop}| (freq_start, freq_stop)).unzip();

    let mut avg_band_powers = Vec::with_capacity(x.len());
    let mut stddev_band_powers = Vec::with_capacity(y.len());

    let res = unsafe {
        data_handler::get_custom_band_powers(
            raw_data.as_mut_ptr() as *mut c_double,
            native_int(rows)?,
            native_int(cols)?,
            x.as_mut_ptr() as *mut c_double,
            y.as_mut_ptr() as *mut c_double,
            native_int(x.len())?,
            native_int(sampling_rate)?,
            apply_filters as c_int,
            avg_band_powers.as_mut_ptr() as *mut c_double,
            stddev_band_powers.as_mut_ptr() as *mut c_double,
        )
    };
    check_brainflow_exit_code(res)?;

    unsafe { avg_band_powers.set_len(x.len()) };
    unsafe { stddev_band_powers.set_len(x.len()) };
    Ok((avg_band_powers, stddev_band_powers))
}

/// Calculate normalized mean powers for bands 2-4, 4-8, 8-13, 13-30, 30-45 Hz.
/// Uses get_custom_band_powers preprocessing, minimum retained length, and variation semantics.
pub fn get_avg_band_powers(
    data: Array2<f64>,
    eeg_channels: Vec<usize>,
    sampling_rate: usize,
    apply_filters: bool,
) -> Result<(Vec<f64>, Vec<f64>)> {
    let vector = vec![
       Band { freq_start: 2.0, freq_stop: 4.0 },
       Band { freq_start: 4.0, freq_stop: 8.0 },
       Band { freq_start: 8.0, freq_stop: 13.0 },
       Band { freq_start: 13.0, freq_stop: 30.0 },
       Band { freq_start: 30.0, freq_stop: 45.0 },
    ];
    get_custom_band_powers(data, vector, eeg_channels, sampling_rate, apply_filters)
}

/// Calculate band power.
pub fn get_band_power(psd: &mut Psd, band: Band) -> Result<f64> {
    let mut band_power = 0.0;
    let res = unsafe {
        data_handler::get_band_power(
            psd.amplitude.as_mut_ptr() as *mut c_double,
            psd.frequency.as_mut_ptr() as *mut c_double,
            native_int(psd.amplitude.len())?,
            band.freq_start,
            band.freq_stop,
            &mut band_power,
        )
    };
    check_brainflow_exit_code(res)?;
    Ok(band_power)
}

/// Calculate nearest power of two.
pub fn get_nearest_power_of_two(value: usize) -> Result<usize> {
    let mut output = 0;
    let res = unsafe { data_handler::get_nearest_power_of_two(native_int(value)?, &mut output) };
    check_brainflow_exit_code(res)?;
    Ok(output as usize)
}

/// Read data from file.
pub fn read_file<S: AsRef<str>>(file_name: S) -> Result<Array2<f64>> {
    let file_name = CString::new(file_name.as_ref())?;
    let mut num_elements = 0;
    let res =
        unsafe { data_handler::get_num_elements_in_file(file_name.as_ptr(), &mut num_elements) };
    check_brainflow_exit_code(res)?;

    let mut data = Vec::with_capacity(num_elements as usize);
    let mut rows = 0;
    let mut cols = 0;
    let res = unsafe {
        data_handler::read_file(
            data.as_mut_ptr() as *mut c_double,
            &mut rows,
            &mut cols,
            file_name.as_ptr(),
            num_elements as c_int,
        )
    };
    check_brainflow_exit_code(res)?;

    if rows <= 0 || cols <= 0 {
        return Err(invalid_arguments());
    }
    let actual_len = checked_product(rows as usize, cols as usize)?;
    if actual_len > num_elements as usize {
        return Err(invalid_arguments());
    }
    unsafe { data.set_len(actual_len) };
    let data = ArrayBase::from_vec(data);
    let data = data.into_shape((rows as usize, cols as usize))?;
    Ok(data)
}

/// Write data to file, in file data will be transposed.
pub fn write_file<S>(data: &Array2<f64>, file_name: S, file_mode: S) -> Result<()>
where
    S: AsRef<str>,
{
    let file_name = CString::new(file_name.as_ref())?;
    let file_mode = CString::new(file_mode.as_ref())?;
    let shape = data.shape();
    let (rows, cols) = (shape[0], shape[1]);
    let mut data: Vec<f64> = data.into_iter().cloned().collect();

    let res = unsafe {
        data_handler::write_file(
            data.as_mut_ptr() as *mut c_double,
            native_int(rows)?,
            native_int(cols)?,
            file_name.as_ptr(),
            file_mode.as_ptr(),
        )
    };
    Ok(check_brainflow_exit_code(res)?)
}

/// Calculate activity index from 3-axis accelerometer data using the Bai et al. (2016) formulation.
pub fn get_activity_index(
    accel_x: &[f64],
    accel_y: &[f64],
    accel_z: &[f64],
    sampling_rate: usize,
    period: Option<usize>,
    noise_var: Option<(f64, f64, f64)>,
) -> Result<Vec<f64>> {
    if accel_x.len() != accel_y.len() || accel_x.len() != accel_z.len() {
        return Err(Error::BrainFlowError(BrainFlowError::InvalidArgumentsError));
    }
    let data_len = accel_x.len();
    if data_len == 0 || sampling_rate == 0 || data_len < sampling_rate {
        return Err(Error::BrainFlowError(BrainFlowError::InvalidArgumentsError));
    }
    let period = period.unwrap_or(data_len - (data_len % sampling_rate));
    if period < sampling_rate || period > data_len || period % sampling_rate != 0 {
        return Err(Error::BrainFlowError(BrainFlowError::InvalidArgumentsError));
    }
    let num_epochs = data_len / period;
    if num_epochs == 0 {
        return Err(Error::BrainFlowError(BrainFlowError::InvalidArgumentsError));
    }
    let (noise_var_x, noise_var_y, noise_var_z) = noise_var.unwrap_or((0.0, 0.0, 0.0));
    if noise_var_x < 0.0 || noise_var_y < 0.0 || noise_var_z < 0.0 {
        return Err(Error::BrainFlowError(BrainFlowError::InvalidArgumentsError));
    }
    let mut output = Vec::<f64>::with_capacity(num_epochs);
    let res = unsafe {
        data_handler::get_activity_index(
            accel_x.as_ptr() as *const c_double,
            accel_y.as_ptr() as *const c_double,
            accel_z.as_ptr() as *const c_double,
            native_int(data_len)?,
            native_int(sampling_rate)?,
            native_int(period)?,
            noise_var_x,
            noise_var_y,
            noise_var_z,
            output.as_mut_ptr() as *mut c_double,
        )
    };
    check_brainflow_exit_code(res)?;

    unsafe { output.set_len(num_epochs) };
    Ok(output)
}

/// Get DataFilter version.
pub fn get_version() -> Result<String> {
    const MAX_CHARS: usize = 64;
    let mut response_len = 0;
    let mut result_char_buffer: [c_char; MAX_CHARS] = [0; MAX_CHARS];
    let (res, response) = unsafe {
        let res = data_handler::get_version_data_handler(result_char_buffer.as_mut_ptr(), &mut response_len, MAX_CHARS as i32);
        let response = CStr::from_ptr(result_char_buffer.as_mut_ptr());
        (res, response)
    };
    check_brainflow_exit_code(res)?;

    Ok(response.to_str()?.to_string())
}

#[cfg(test)]
mod tests {
    use std::{env, f64::consts::PI, fs};
    use ndarray::array;
    use crate::ffi::constants::WindowOperations;
    use crate::test_helpers::assertions::assert_regex_matches;
    use crate::test_helpers::consts::VERSION_PATTERN;
    use super::*;

    #[test]
    fn test_it_gets_the_version() {
        assert_regex_matches(VERSION_PATTERN, get_version().unwrap().as_str());
    }

    #[test]
    fn wavelet_inverse_transform_equals_input_data() {
        let step = 2.0 * PI / 256.0;
        let mut data = vec![];
        let mut value = -PI;
        for _ in 0..256 {
            data.push(value.sin());
            value += step;
        }

        let fft_data = perform_fft(&mut data, WindowOperations::BlackmanHarris).unwrap();
        let restored_fft = perform_ifft(&fft_data, data.len()).unwrap();
        println!("{:?}", restored_fft);

        println!("{:?}", data);
        let wavelet_data = perform_wavelet_transform(&mut data, WaveletTypes::Db3, 3, WaveletExtensionTypes::Periodic).unwrap();
        assert_eq!(wavelet_data.decomposition_lengths().len(), 4);
        assert_eq!(wavelet_data.coefficients().len(), wavelet_data.decomposition_lengths().iter().sum::<usize>());
        let restored_wavelet = perform_inverse_wavelet_transform(wavelet_data.clone()).unwrap();
        println!("{:?}", restored_wavelet);
        for (d, r) in data.iter().zip(restored_wavelet) {
            assert_relative_eq!(*d, r, max_relative = 1e-14);
        }
    }

    #[test]
    fn rejects_mismatched_buffers_before_native_access() {
        let mut short = vec![1.0; 4];
        let mut long = vec![1.0; 8];
        assert!(calc_stddev(&mut short, 0, 8).is_err());
        assert!(get_railed_percentage(&mut short, 8, 24).is_err());
        assert!(get_heart_rate(&mut short, &mut long, 64, 1024).is_err());
        assert!(get_oxygen_level(&mut short, &mut long, 64, 1.0, 1.0, 1.0).is_err());
        assert!(perform_ifft(&[Complex64::new(1.0, 0.0); 3], 8).is_err());
        assert!(get_csp(&Array3::zeros((2, 2, 8)), &Array1::zeros(1)).is_err());
        assert!(native_int(c_int::MAX as usize + 1).is_err());
        let invalid = WaveletTransform::with_coefficients(vec![0.0; 4], 2, vec![2, 2],
            WaveletTypes::Db3, WaveletExtensionTypes::Periodic, 8);
        assert!(perform_inverse_wavelet_transform(invalid).is_err());
        for original_length in [4, c_int::MAX as usize / 2, c_int::MAX as usize] {
            let oversized = WaveletTransform::with_coefficients(vec![0.0; 2], 1, vec![1, 1],
                WaveletTypes::Haar, WaveletExtensionTypes::Periodic, original_length);
            assert!(perform_inverse_wavelet_transform(oversized).is_err());
        }
    }

    #[test]
    fn channel_selection_preserves_order_and_duplicates() {
        let data = array![[1.0, 2.0], [3.0, 4.0], [5.0, 6.0]];
        assert_eq!(selected_data(&data, &[2, 0, 2]).unwrap(), vec![5.0, 6.0, 1.0, 2.0, 5.0, 6.0]);
        assert!(selected_data(&data, &[0, 3]).is_err());
    }

    #[test]
    fn ica_returns_initialized_output_vectors() {
        let data = Array2::from_shape_fn((2, 1024), |(row, col)| {
            let t = col as f64 / 256.0;
            let first = (2.0 * PI * 7.0 * t).sin();
            let second = (2.0 * PI * 13.0 * t).sin().powi(3);
            if row == 0 { first + 0.3 * second } else { 0.2 * first + second }
        });
        let (w, k, a, s) = perform_ica_select_channels(data, 2, vec![1, 0]).unwrap();
        assert_eq!((w.len(), k.len(), a.len(), s.len()), (4, 4, 4, 2048));
        assert!(w.iter().chain(&k).chain(&a).chain(&s).all(|v| v.is_finite()));
    }

    #[test]
    fn read_written_data_is_same_as_input() {
        let data = array![[1.0, 2.0, 3.0], [4.0, 5.0, 6.0]];

        let mut tmp_dir = env::temp_dir();
        tmp_dir.push("brainflow_tests");
        tmp_dir.push("rust");
        fs::create_dir_all(&tmp_dir).unwrap();
        tmp_dir.push("read_written_data_is_same_as_input.csv");
        let filename = tmp_dir.to_str().unwrap();

        write_file(&data, filename, "w").unwrap();
        let read_data = read_file(filename).unwrap();
        assert_eq!(data, read_data);
    }
}
