use getset::Getters;
use ndarray::{Array1, Array2, Array3, ArrayBase};
use num::Complex;
use num_complex::Complex64;
use std::os::raw::c_int;
use std::{ffi::CString, os::raw::c_double};

use crate::error::{BrainFlowError, Error};
use crate::ffi::data_handler;
use crate::{
    check_brainflow_exit_code, AggOperations, DetrendOperations, FilterTypes, LogLevels,
    NoiseTypes, Result, WindowFunctions,
};

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

/// Apply band pass filter to provided data.
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
        data_handler::perform_bandpass(
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

/// Apply band stop filter to provided data.
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
        data_handler::perform_bandstop(
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

/// Remove environmantal noise using notch filter.
pub fn remove_environmental_noise(
    data: &mut [f64],
    sampling_rate: usize,
    noise_type: NoiseTypes,
) -> Result<()> {
    let res = unsafe {
        data_handler::remove_environmental_noise(
            data.as_mut_ptr() as *mut c_double,
            data.len() as c_int,
            sampling_rate as c_int,
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
            data.len() as c_int,
            period as c_int,
            agg_operation as c_int,
        )
    };
    check_brainflow_exit_code(res)?;
    Ok(())
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
            data.len() as c_int,
            period as c_int,
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

    /// Create new WaveletTransform with coefficients.
    /// This function can be used to create input data for [perform_inverse_wavelet_transform].
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

/// Perform wavelet transform.
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
        data_handler::perform_wavelet_transform(
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

/// Perform inverse wavelet transform.
pub fn perform_inverse_wavelet_transform(wavelet_transform: WaveletTransform) -> Result<Vec<f64>> {
    let mut wavelet_transform = wavelet_transform;
    let mut output = Vec::<f64>::with_capacity(wavelet_transform.original_data_len);
    let wavelet = CString::new(wavelet_transform.wavelet)?;
    let res = unsafe {
        data_handler::perform_inverse_wavelet_transform(
            wavelet_transform.coefficients.as_mut_ptr() as *mut c_double,
            wavelet_transform.original_data_len as c_int,
            wavelet.as_ptr(),
            wavelet_transform.decomposition_level as c_int,
            wavelet_transform.decomposition_lengths.as_ptr() as *mut c_int,
            output.as_mut_ptr() as *mut c_double,
        )
    };
    check_brainflow_exit_code(res)?;
    unsafe { output.set_len(wavelet_transform.original_data_len) }
    Ok(output)
}

/// Perform wavelet denoising.
pub fn perform_wavelet_denoising<S: AsRef<str>>(
    data: &mut [f64],
    wavelet: S,
    decomposition_level: usize,
) -> Result<()> {
    let wavelet = CString::new(wavelet.as_ref())?;
    let res = unsafe {
        data_handler::perform_wavelet_denoising(
            data.as_mut_ptr() as *mut c_double,
            data.len() as c_int,
            wavelet.as_ptr(),
            decomposition_level as c_int,
        )
    };
    check_brainflow_exit_code(res)?;
    Ok(())
}

/// Calculate filters and the corresponding eigenvalues using the Common Spatial Patterns.
pub fn get_csp<Labels>(
    data: &Array3<f64>,
    labels: &Array1<f64>,
) -> Result<(Array2<f64>, Array1<f64>)> {
    let shape = data.shape();
    let n_epochs = shape[0];
    let n_channels = shape[1];
    let n_times = shape[2];
    let data: Vec<f64> = data.into_iter().cloned().collect();

    let labels: Vec<f64> = labels.into_iter().cloned().collect();

    let mut output_filters = Vec::<f64>::with_capacity(n_channels * n_channels);
    let mut output_eigenvalues = Vec::<f64>::with_capacity(n_channels);

    let res = unsafe {
        data_handler::get_csp(
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

    unsafe { output_filters.set_len(n_channels * n_channels) };
    unsafe { output_eigenvalues.set_len(n_channels) };

    let output_filters = ArrayBase::from_vec(output_filters);
    let output_filters = output_filters.into_shape((n_channels, n_channels)).unwrap();
    let output_eigenvalues = Array1::from(output_eigenvalues);
    Ok((output_filters, output_eigenvalues))
}

/// Perform data windowing.
pub fn get_window(window_function: WindowFunctions, window_len: usize) -> Result<Vec<f64>> {
    let mut output = Vec::<f64>::with_capacity(window_len);
    let res = unsafe {
        data_handler::get_window(
            window_function as c_int,
            window_len as c_int,
            output.as_mut_ptr() as *mut c_double,
        )
    };
    check_brainflow_exit_code(res)?;

    unsafe { output.set_len(window_len) };
    Ok(output)
}

/// Perform direct FFT.
pub fn perform_fft(data: &mut [f64], window_function: WindowFunctions) -> Result<Vec<Complex64>> {
    let mut output_re = Vec::<f64>::with_capacity(data.len() / 2 + 1);
    let mut output_im = Vec::<f64>::with_capacity(data.len() / 2 + 1);
    let res = unsafe {
        data_handler::perform_fft(
            data.as_mut_ptr() as *mut c_double,
            data.len() as c_int,
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
    let mut restored_data = Vec::<f64>::with_capacity(original_data_len);
    let (mut input_re, mut input_im): (Vec<f64>, Vec<f64>) =
        data.iter().map(|d| (d.re, d.im)).unzip();
    let res = unsafe {
        data_handler::perform_ifft(
            input_re.as_mut_ptr() as *mut c_double,
            input_im.as_mut_ptr() as *mut c_double,
            original_data_len as c_int,
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
            data.len() as c_int,
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
    window_function: WindowFunctions,
) -> Result<Psd> {
    let mut amplitude = Vec::<f64>::with_capacity(data.len() / 2 + 1);
    let mut frequency = Vec::<f64>::with_capacity(data.len() / 2 + 1);
    let res = unsafe {
        data_handler::get_psd(
            data.as_mut_ptr() as *mut c_double,
            data.len() as c_int,
            sampling_rate as c_int,
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
    window_function: WindowFunctions,
) -> Result<Psd> {
    let mut amplitude = Vec::<f64>::with_capacity(nfft / 2 + 1);
    let mut frequency = Vec::<f64>::with_capacity(nfft / 2 + 1);
    let res = unsafe {
        data_handler::get_psd_welch(
            data.as_mut_ptr() as *mut c_double,
            data.len() as c_int,
            nfft as c_int,
            overlap as c_int,
            sampling_rate as c_int,
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

/// Calculate avg and stddev of BandPowers across all channels, bands are 1-4,4-8,8-13,13-30,30-50.
pub fn get_avg_band_powers(
    data: Array2<f64>,
    eeg_channels: Vec<usize>,
    sampling_rate: usize,
    apply_filters: bool,
) -> Result<(Vec<f64>, Vec<f64>)> {
    let shape = data.shape();
    let (rows, cols) = (eeg_channels.len(), shape[1]);
    let mut raw_data = data
        .outer_iter()
        .enumerate()
        .filter(|(i, _)| eeg_channels.contains(i))
        .map(|(_, x)| x)
        .flatten()
        .copied()
        .collect::<Vec<f64>>();

    let mut avg_band_powers = Vec::with_capacity(5);
    let mut stddev_band_powers = Vec::with_capacity(5);

    let res = unsafe {
        data_handler::get_avg_band_powers(
            raw_data.as_mut_ptr() as *mut c_double,
            rows as c_int,
            cols as c_int,
            sampling_rate as c_int,
            apply_filters as c_int,
            avg_band_powers.as_mut_ptr() as *mut c_double,
            stddev_band_powers.as_mut_ptr() as *mut c_double,
        )
    };
    check_brainflow_exit_code(res)?;

    unsafe { avg_band_powers.set_len(5) };
    unsafe { stddev_band_powers.set_len(5) };
    Ok((avg_band_powers, stddev_band_powers))
}

/// Calculate band power.
pub fn get_band_power(psd: &mut Psd, freq_start: f64, freq_end: f64) -> Result<f64> {
    let mut band_power = 0.0;
    let res = unsafe {
        data_handler::get_band_power(
            psd.amplitude.as_mut_ptr() as *mut c_double,
            psd.frequency.as_mut_ptr() as *mut c_double,
            psd.amplitude.len() as c_int,
            freq_start,
            freq_end,
            &mut band_power,
        )
    };
    check_brainflow_exit_code(res)?;
    Ok(band_power)
}

/// Calculate nearest power of two.
pub fn get_nearest_power_of_two(value: usize) -> Result<usize> {
    let mut output = 0;
    let res = unsafe { data_handler::get_nearest_power_of_two(value as c_int, &mut output) };
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

    unsafe { data.set_len(num_elements as usize) };
    let data = ArrayBase::from_vec(data);
    let data = data.into_shape((rows as usize, cols as usize)).unwrap();
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
            rows as c_int,
            cols as c_int,
            file_name.as_ptr(),
            file_mode.as_ptr(),
        )
    };
    Ok(check_brainflow_exit_code(res)?)
}

#[cfg(test)]
mod tests {
    use std::{env, f64::consts::PI, fs};

    use ndarray::array;

    use crate::ffi::constants::WindowFunctions;

    use super::*;

    #[test]
    fn wavelet_inverse_transform_equals_input_data() {
        let step = 2.0 * PI / 256.0;
        let mut data = vec![];
        let mut value = -PI;
        for _ in 0..256 {
            data.push(value.sin());
            value += step;
        }

        let fft_data = perform_fft(&mut data, WindowFunctions::BlackmanHarris).unwrap();
        let restored_fft = perform_ifft(&fft_data, data.len()).unwrap();
        println!("{:?}", restored_fft);

        println!("{:?}", data);
        let wavelet_data = perform_wavelet_transform(&mut data, "db3", 3).unwrap();
        let restored_wavelet = perform_inverse_wavelet_transform(wavelet_data).unwrap();
        println!("{:?}", restored_wavelet);
        for (d, r) in data.iter().zip(restored_wavelet) {
            assert_relative_eq!(*d, r, max_relative = 1e-14);
        }
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
