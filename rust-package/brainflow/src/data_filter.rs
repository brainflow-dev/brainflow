use getset::Getters;
use ndarray::{Array1, Array2, ArrayBase, AsArray, Ix1, Ix3};
use num::Complex;
use num_complex::Complex64;
use std::os::raw::c_int;
use std::{ffi::CString, os::raw::c_double};

use ffi::{AggOperations, DetrendOperations, FilterTypes, LogLevels, NoiseTypes, WindowFunctions};

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
    data: &mut [f64],
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
    check_brainflow_exit_code(res)?;
    Ok(())
}

pub fn perform_highpass(
    data: &mut [f64],
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
    check_brainflow_exit_code(res)?;
    Ok(())
}

pub fn perform_bandpass(
    data: &mut [f64],
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
    check_brainflow_exit_code(res)?;
    Ok(())
}

pub fn perform_bandstop(
    data: &mut [f64],
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
    check_brainflow_exit_code(res)?;
    Ok(())
}

pub fn remove_environmental_noise(
    data: &mut [f64],
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
    check_brainflow_exit_code(res)?;
    Ok(())
}

pub fn perform_rolling_filter(
    data: &mut [f64],
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
    check_brainflow_exit_code(res)?;
    Ok(())
}

pub fn perform_downsampling(
    data: &mut [f64],
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

#[derive(Getters)]
#[getset(get = "pub")]
pub struct WaveletTransform {
    coefficients: Vec<f64>,
    decomposition_level: usize,
    decomposition_lengths: Vec<usize>,
    wavelet: String,
    original_data_len: usize,
}

impl WaveletTransform {
    pub fn new(
        capacity: usize,
        decomposition_level: usize,
        wavelet: String,
        original_data_len: usize,
    ) -> Self {
        Self {
            coefficients: Vec::with_capacity(capacity),
            decomposition_level,
            decomposition_lengths: Vec::with_capacity(decomposition_level + 1),
            wavelet,
            original_data_len,
        }
    }

    pub fn with_coefficients(
        coefficients: Vec<f64>,
        decomposition_level: usize,
        decomposition_lengths: Vec<usize>,
        wavelet: String,
        original_data_len: usize,
    ) -> Self {
        Self {
            coefficients,
            decomposition_level,
            decomposition_lengths,
            wavelet,
            original_data_len,
        }
    }
}

pub fn perform_wavelet_transform<S: AsRef<str>>(
    data: &mut [f64],
    wavelet: S,
    decomposition_level: usize,
) -> Result<WaveletTransform> {
    let capacity = data.len() + 2 * decomposition_level * (40 + 1);
    let mut wavelet_transform = WaveletTransform::new(
        capacity,
        decomposition_level,
        wavelet.as_ref().to_string(),
        data.len(),
    );
    let wavelet = CString::new(wavelet.as_ref())?;
    let res = unsafe {
        let output = wavelet_transform.coefficients.as_mut_ptr() as *mut c_double;
        let decomposition_lengths =
            wavelet_transform.decomposition_lengths.as_mut_ptr() as *mut c_int;
        ffi::perform_wavelet_transform(
            data.as_mut_ptr() as *mut c_double,
            data.len() as c_int,
            wavelet.as_ptr(),
            decomposition_level as c_int,
            output,
            decomposition_lengths,
        )
    };
    check_brainflow_exit_code(res)?;
    Ok(wavelet_transform)
}

pub fn perform_inverse_wavelet_transform(wavelet_transform: WaveletTransform) -> Result<Vec<f64>> {
    let mut wavelet_transform = wavelet_transform;
    let mut output = Vec::<f64>::with_capacity(wavelet_transform.original_data_len);
    let wavelet = CString::new(wavelet_transform.wavelet)?;
    let res = unsafe {
        ffi::perform_inverse_wavelet_transform(
            wavelet_transform.coefficients.as_mut_ptr() as *mut c_double,
            wavelet_transform.original_data_len as c_int,
            wavelet.as_ptr(),
            wavelet_transform.decomposition_level as c_int,
            wavelet_transform.decomposition_lengths.as_ptr() as *mut c_int,
            output.as_mut_ptr() as *mut c_double,
        )
    };
    check_brainflow_exit_code(res)?;
    Ok(output)
}

pub fn perform_wavelet_denoising<S: AsRef<str>>(
    data: &mut [f64],
    wavelet: S,
    decomposition_level: usize,
) -> Result<()> {
    let wavelet = CString::new(wavelet.as_ref())?;
    let res = unsafe {
        ffi::perform_wavelet_denoising(
            data.as_mut_ptr() as *mut c_double,
            data.len() as c_int,
            wavelet.as_ptr(),
            decomposition_level as c_int,
        )
    };
    check_brainflow_exit_code(res)?;
    Ok(())
}

pub fn csp<'a, Data, Labels>(data: Data, labels: Labels) -> Result<(Array2<f64>, Array1<f64>)>
where
    Data: AsArray<'a, f64, Ix3>,
    Labels: AsArray<'a, f64, Ix1>,
{
    let data = data.into();
    let shape = data.shape();
    let n_epochs = shape[0];
    let n_channels = shape[1];
    let n_times = shape[2];
    let data: Vec<&f64> = data.iter().collect();

    let labels = labels.into();
    let labels: Vec<&f64> = labels.iter().collect();

    let mut output_filters = Vec::<f64>::with_capacity(n_channels * n_channels);
    let mut output_eigenvalues = Vec::<f64>::with_capacity(n_channels);

    let res = unsafe {
        ffi::get_csp(
            data.as_ptr() as *const c_double,
            labels.as_ptr() as *const c_double,
            n_epochs as c_int,
            n_channels as c_int,
            n_times as c_int,
            output_filters.as_mut_ptr() as *mut c_double,
            output_eigenvalues.as_mut_ptr() as *mut c_double,
        )
    };
    check_brainflow_exit_code(res)?;

    let output_filters = ArrayBase::from_vec(output_filters);
    let output_filters = output_filters.into_shape((n_channels, n_channels)).unwrap();
    let output_eigenvalues = Array1::from(output_eigenvalues);
    Ok((output_filters, output_eigenvalues))
}

pub fn get_csp<'a, Data, Labels>(data: Data, labels: Labels) -> Result<(Array2<f64>, Array1<f64>)>
where
    Data: AsArray<'a, f64, Ix3>,
    Labels: AsArray<'a, f64, Ix1>,
{
    csp(data, labels)
}

pub fn window(window_function: WindowFunctions, window_len: usize) -> Result<Vec<f64>> {
    let mut output = Vec::<f64>::with_capacity(window_len);
    let res = unsafe {
        ffi::get_window(
            window_function as c_int,
            window_len as c_int,
            output.as_mut_ptr() as *mut c_double,
        )
    };
    check_brainflow_exit_code(res)?;
    Ok(output)
}

pub fn get_window(window_function: WindowFunctions, window_len: usize) -> Result<Vec<f64>> {
    window(window_function, window_len)
}

pub fn perform_fft(data: &mut [f64], window_function: WindowFunctions) -> Result<Vec<Complex64>> {
    let mut output_re = Vec::<f64>::with_capacity(data.len() / 2 + 1);
    let mut output_im = Vec::<f64>::with_capacity(data.len() / 2 + 1);
    let res = unsafe {
        ffi::perform_fft(
            data.as_mut_ptr() as *mut c_double,
            data.len() as c_int,
            window_function as c_int,
            output_re.as_mut_ptr() as *mut c_double,
            output_im.as_mut_ptr() as *mut c_double,
        )
    };
    let output = output_re
        .into_iter()
        .zip(output_im)
        .map(|(re, im)| Complex { re, im })
        .collect();
    check_brainflow_exit_code(res)?;
    Ok(output)
}

pub fn perform_ifft(data: &[Complex64], original_data_len: usize) -> Result<Vec<f64>> {
    let mut restored_data = Vec::<f64>::with_capacity(original_data_len);
    let (mut input_re, mut input_im): (Vec<f64>, Vec<f64>) =
        data.iter().map(|d| (d.re, d.im)).unzip();
    let res = unsafe {
        ffi::perform_ifft(
            input_re.as_mut_ptr() as *mut c_double,
            input_im.as_mut_ptr() as *mut c_double,
            original_data_len as c_int,
            restored_data.as_mut_ptr() as *mut c_double,
        )
    };
    check_brainflow_exit_code(res)?;
    Ok(restored_data)
}

pub fn detrend(data: &mut [f64], detrend_operation: DetrendOperations) -> Result<()> {
    let res = unsafe {
        ffi::detrend(
            data.as_mut_ptr() as *mut c_double,
            data.len() as c_int,
            detrend_operation as c_int,
        )
    };
    Ok(check_brainflow_exit_code(res)?)
}

pub fn psd(
    data: &mut [f64],
    sampling_rate: usize,
    window_function: WindowFunctions,
) -> Result<(Vec<f64>, Vec<f64>)> {
    let mut output_ampl = Vec::<f64>::with_capacity(data.len() / 2 + 1);
    let mut output_freq = Vec::<f64>::with_capacity(data.len() / 2 + 1);
    let res = unsafe {
        ffi::get_psd(
            data.as_mut_ptr() as *mut c_double,
            data.len() as c_int,
            sampling_rate as c_int,
            window_function as c_int,
            output_ampl.as_mut_ptr() as *mut c_double,
            output_freq.as_mut_ptr() as *mut c_double,
        )
    };
    check_brainflow_exit_code(res)?;
    Ok((output_ampl, output_freq))
}
