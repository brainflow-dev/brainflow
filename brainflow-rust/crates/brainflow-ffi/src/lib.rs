//! C FFI bindings for BrainFlow Rust implementation.
//!
//! This crate provides C-compatible functions that can be called from
//! Python, Java, C#, Julia, MATLAB, and other languages.
//!
//! The API is designed to be ABI-compatible with the original C++ BrainFlow
//! library, allowing existing bindings to work with minimal changes.

use std::ffi::{c_char, c_double, c_int, CStr};
use std::ptr;
use std::slice;
use std::sync::Mutex;

use brainflow_boards::{BoardController, BoardId, BoardParams, Preset};
use brainflow_fft::window::WindowType;
use brainflow_filter::{DetrendType, FilterType, NoiseType};
use brainflow_wavelet::denoise::{DenoiseMethod, ThresholdType};
use brainflow_wavelet::wavelets::Wavelet;

// ============================================================================
// Error codes (matching C++ BrainFlow)
// ============================================================================

/// Status OK.
pub const STATUS_OK: c_int = 0;
/// Unknown error.
pub const GENERAL_ERROR: c_int = 1;
/// Invalid arguments.
pub const INVALID_ARGUMENTS_ERROR: c_int = 2;
/// Board not ready.
pub const BOARD_NOT_READY_ERROR: c_int = 3;
/// Stream thread not running.
pub const STREAM_THREAD_NOT_RUNNING_ERROR: c_int = 4;
/// Empty buffer.
pub const EMPTY_BUFFER_ERROR: c_int = 5;
/// Board not created.
pub const BOARD_NOT_CREATED_ERROR: c_int = 6;
/// Another board created.
pub const ANOTHER_BOARD_IS_CREATED_ERROR: c_int = 7;
/// Unsupported board.
pub const UNSUPPORTED_BOARD_ERROR: c_int = 8;

// ============================================================================
// Global state
// ============================================================================

lazy_static::lazy_static! {
    static ref CONTROLLER: Mutex<BoardController> = Mutex::new(BoardController::new());
    static ref LAST_ERROR: Mutex<String> = Mutex::new(String::new());
}

/// Store error message for retrieval.
fn set_error(msg: &str) {
    if let Ok(mut err) = LAST_ERROR.lock() {
        *err = msg.to_string();
    }
}

// ============================================================================
// Board control functions
// ============================================================================

/// Prepare a streaming session.
///
/// # Safety
/// `params_json` must be a valid null-terminated C string.
#[no_mangle]
pub unsafe extern "C" fn brainflow_prepare_session(
    board_id: c_int,
    params_json: *const c_char,
) -> c_int {
    let board_id = match board_id_from_int(board_id) {
        Some(id) => id,
        None => {
            set_error("Invalid board ID");
            return UNSUPPORTED_BOARD_ERROR;
        }
    };

    let params = if params_json.is_null() {
        BoardParams::new()
    } else {
        match parse_params_json(params_json) {
            Ok(p) => p,
            Err(e) => {
                set_error(&e);
                return INVALID_ARGUMENTS_ERROR;
            }
        }
    };

    let controller = match CONTROLLER.lock() {
        Ok(c) => c,
        Err(_) => {
            set_error("Failed to acquire lock");
            return GENERAL_ERROR;
        }
    };

    match controller.prepare_session(board_id, &params) {
        Ok(()) => STATUS_OK,
        Err(e) => {
            set_error(&format!("{:?}", e));
            error_to_code(&e)
        }
    }
}

/// Release a streaming session.
#[no_mangle]
pub unsafe extern "C" fn brainflow_release_session(
    board_id: c_int,
    params_json: *const c_char,
) -> c_int {
    let board_id = match board_id_from_int(board_id) {
        Some(id) => id,
        None => return UNSUPPORTED_BOARD_ERROR,
    };

    let params = if params_json.is_null() {
        BoardParams::new()
    } else {
        match parse_params_json(params_json) {
            Ok(p) => p,
            Err(_) => return INVALID_ARGUMENTS_ERROR,
        }
    };

    let controller = match CONTROLLER.lock() {
        Ok(c) => c,
        Err(_) => return GENERAL_ERROR,
    };

    match controller.release_session(board_id, &params) {
        Ok(()) => STATUS_OK,
        Err(e) => error_to_code(&e),
    }
}

/// Start data streaming.
#[no_mangle]
pub unsafe extern "C" fn brainflow_start_stream(
    board_id: c_int,
    buffer_size: c_int,
    params_json: *const c_char,
) -> c_int {
    let board_id = match board_id_from_int(board_id) {
        Some(id) => id,
        None => return UNSUPPORTED_BOARD_ERROR,
    };

    let params = if params_json.is_null() {
        BoardParams::new()
    } else {
        match parse_params_json(params_json) {
            Ok(p) => p,
            Err(_) => return INVALID_ARGUMENTS_ERROR,
        }
    };

    let controller = match CONTROLLER.lock() {
        Ok(c) => c,
        Err(_) => return GENERAL_ERROR,
    };

    let buffer_size = if buffer_size <= 0 { 450000 } else { buffer_size as usize };

    match controller.start_stream(board_id, &params, buffer_size) {
        Ok(()) => STATUS_OK,
        Err(e) => error_to_code(&e),
    }
}

/// Stop data streaming.
#[no_mangle]
pub unsafe extern "C" fn brainflow_stop_stream(
    board_id: c_int,
    params_json: *const c_char,
) -> c_int {
    let board_id = match board_id_from_int(board_id) {
        Some(id) => id,
        None => return UNSUPPORTED_BOARD_ERROR,
    };

    let params = if params_json.is_null() {
        BoardParams::new()
    } else {
        match parse_params_json(params_json) {
            Ok(p) => p,
            Err(_) => return INVALID_ARGUMENTS_ERROR,
        }
    };

    let controller = match CONTROLLER.lock() {
        Ok(c) => c,
        Err(_) => return GENERAL_ERROR,
    };

    match controller.stop_stream(board_id, &params) {
        Ok(()) => STATUS_OK,
        Err(e) => error_to_code(&e),
    }
}

/// Get current board data (peek, doesn't remove from buffer).
///
/// # Safety
/// `data_out` must point to a buffer of at least `num_samples * num_channels` doubles.
#[no_mangle]
pub unsafe extern "C" fn brainflow_get_current_board_data(
    board_id: c_int,
    num_samples: c_int,
    preset: c_int,
    params_json: *const c_char,
    data_out: *mut c_double,
    num_samples_out: *mut c_int,
) -> c_int {
    let board_id = match board_id_from_int(board_id) {
        Some(id) => id,
        None => return UNSUPPORTED_BOARD_ERROR,
    };

    let params = if params_json.is_null() {
        BoardParams::new()
    } else {
        match parse_params_json(params_json) {
            Ok(p) => p,
            Err(_) => return INVALID_ARGUMENTS_ERROR,
        }
    };

    let preset = preset_from_int(preset);

    let controller = match CONTROLLER.lock() {
        Ok(c) => c,
        Err(_) => return GENERAL_ERROR,
    };

    match controller.get_current_board_data(board_id, &params, num_samples as usize, preset) {
        Ok(data) => {
            if !data_out.is_null() {
                ptr::copy_nonoverlapping(data.as_ptr(), data_out, data.len());
            }
            if !num_samples_out.is_null() {
                *num_samples_out = data.len() as c_int;
            }
            STATUS_OK
        }
        Err(e) => error_to_code(&e),
    }
}

/// Get board data (removes from buffer).
///
/// # Safety
/// `data_out` must point to a buffer of at least `num_samples * num_channels` doubles.
#[no_mangle]
pub unsafe extern "C" fn brainflow_get_board_data(
    board_id: c_int,
    num_samples: c_int,
    preset: c_int,
    params_json: *const c_char,
    data_out: *mut c_double,
    num_samples_out: *mut c_int,
) -> c_int {
    let board_id = match board_id_from_int(board_id) {
        Some(id) => id,
        None => return UNSUPPORTED_BOARD_ERROR,
    };

    let params = if params_json.is_null() {
        BoardParams::new()
    } else {
        match parse_params_json(params_json) {
            Ok(p) => p,
            Err(_) => return INVALID_ARGUMENTS_ERROR,
        }
    };

    let preset = preset_from_int(preset);
    let num_samples = if num_samples <= 0 { None } else { Some(num_samples as usize) };

    let controller = match CONTROLLER.lock() {
        Ok(c) => c,
        Err(_) => return GENERAL_ERROR,
    };

    match controller.get_board_data(board_id, &params, num_samples, preset) {
        Ok(data) => {
            if !data_out.is_null() {
                ptr::copy_nonoverlapping(data.as_ptr(), data_out, data.len());
            }
            if !num_samples_out.is_null() {
                *num_samples_out = data.len() as c_int;
            }
            STATUS_OK
        }
        Err(e) => error_to_code(&e),
    }
}

/// Get the number of samples in the buffer.
#[no_mangle]
pub unsafe extern "C" fn brainflow_get_board_data_count(
    board_id: c_int,
    preset: c_int,
    params_json: *const c_char,
    count_out: *mut c_int,
) -> c_int {
    let board_id = match board_id_from_int(board_id) {
        Some(id) => id,
        None => return UNSUPPORTED_BOARD_ERROR,
    };

    let params = if params_json.is_null() {
        BoardParams::new()
    } else {
        match parse_params_json(params_json) {
            Ok(p) => p,
            Err(_) => return INVALID_ARGUMENTS_ERROR,
        }
    };

    let preset = preset_from_int(preset);

    let controller = match CONTROLLER.lock() {
        Ok(c) => c,
        Err(_) => return GENERAL_ERROR,
    };

    match controller.get_board_data_count(board_id, &params, preset) {
        Ok(count) => {
            if !count_out.is_null() {
                *count_out = count as c_int;
            }
            STATUS_OK
        }
        Err(e) => error_to_code(&e),
    }
}

/// Insert a marker into the data stream.
#[no_mangle]
pub unsafe extern "C" fn brainflow_insert_marker(
    board_id: c_int,
    value: c_double,
    preset: c_int,
    params_json: *const c_char,
) -> c_int {
    let board_id = match board_id_from_int(board_id) {
        Some(id) => id,
        None => return UNSUPPORTED_BOARD_ERROR,
    };

    let params = if params_json.is_null() {
        BoardParams::new()
    } else {
        match parse_params_json(params_json) {
            Ok(p) => p,
            Err(_) => return INVALID_ARGUMENTS_ERROR,
        }
    };

    let preset = preset_from_int(preset);

    let controller = match CONTROLLER.lock() {
        Ok(c) => c,
        Err(_) => return GENERAL_ERROR,
    };

    match controller.insert_marker(board_id, &params, value, preset) {
        Ok(()) => STATUS_OK,
        Err(e) => error_to_code(&e),
    }
}

/// Check if board is prepared.
#[no_mangle]
pub unsafe extern "C" fn brainflow_is_prepared(
    board_id: c_int,
    params_json: *const c_char,
    is_prepared: *mut c_int,
) -> c_int {
    let board_id = match board_id_from_int(board_id) {
        Some(id) => id,
        None => return UNSUPPORTED_BOARD_ERROR,
    };

    let params = if params_json.is_null() {
        BoardParams::new()
    } else {
        match parse_params_json(params_json) {
            Ok(p) => p,
            Err(_) => return INVALID_ARGUMENTS_ERROR,
        }
    };

    let controller = match CONTROLLER.lock() {
        Ok(c) => c,
        Err(_) => return GENERAL_ERROR,
    };

    let prepared = controller.is_prepared(board_id, &params);
    if !is_prepared.is_null() {
        *is_prepared = if prepared { 1 } else { 0 };
    }

    STATUS_OK
}

/// Get sampling rate for a board.
#[no_mangle]
pub unsafe extern "C" fn brainflow_get_sampling_rate(
    board_id: c_int,
    preset: c_int,
    params_json: *const c_char,
    sampling_rate: *mut c_int,
) -> c_int {
    let board_id = match board_id_from_int(board_id) {
        Some(id) => id,
        None => return UNSUPPORTED_BOARD_ERROR,
    };

    let params = if params_json.is_null() {
        BoardParams::new()
    } else {
        match parse_params_json(params_json) {
            Ok(p) => p,
            Err(_) => return INVALID_ARGUMENTS_ERROR,
        }
    };

    let preset = preset_from_int(preset);

    let controller = match CONTROLLER.lock() {
        Ok(c) => c,
        Err(_) => return GENERAL_ERROR,
    };

    match controller.get_board_description(board_id, &params, preset) {
        Ok(desc) => {
            if !sampling_rate.is_null() {
                *sampling_rate = desc.sampling_rate as c_int;
            }
            STATUS_OK
        }
        Err(e) => error_to_code(&e),
    }
}

// ============================================================================
// Signal processing functions
// ============================================================================

/// Perform FFT.
///
/// # Safety
/// `data` must point to `data_len` doubles. `real_out` and `imag_out` must
/// point to buffers of at least `data_len` doubles.
#[no_mangle]
pub unsafe extern "C" fn brainflow_perform_fft(
    data: *const c_double,
    data_len: c_int,
    window_type: c_int,
    real_out: *mut c_double,
    imag_out: *mut c_double,
) -> c_int {
    if data.is_null() || data_len <= 0 {
        return INVALID_ARGUMENTS_ERROR;
    }

    let data_slice = slice::from_raw_parts(data, data_len as usize);
    let window = window_from_int(window_type);

    let (real, imag) = brainflow_fft::perform_fft(data_slice, window);

    if !real_out.is_null() {
        ptr::copy_nonoverlapping(real.as_ptr(), real_out, real.len());
    }
    if !imag_out.is_null() {
        ptr::copy_nonoverlapping(imag.as_ptr(), imag_out, imag.len());
    }

    STATUS_OK
}

/// Perform inverse FFT.
///
/// # Safety
/// `real` and `imag` must point to `data_len` doubles each.
/// `data_out` must point to a buffer of at least `data_len` doubles.
#[no_mangle]
pub unsafe extern "C" fn brainflow_perform_ifft(
    real: *const c_double,
    imag: *const c_double,
    data_len: c_int,
    data_out: *mut c_double,
) -> c_int {
    if real.is_null() || imag.is_null() || data_len <= 0 {
        return INVALID_ARGUMENTS_ERROR;
    }

    let real_slice = slice::from_raw_parts(real, data_len as usize);
    let imag_slice = slice::from_raw_parts(imag, data_len as usize);

    let result = brainflow_fft::perform_ifft(real_slice, imag_slice);

    if !data_out.is_null() {
        ptr::copy_nonoverlapping(result.as_ptr(), data_out, result.len());
    }

    STATUS_OK
}

/// Get PSD (power spectral density).
///
/// # Safety
/// `data` must point to `data_len` doubles.
/// `psd_out` and `freq_out` must point to buffers of at least `data_len/2+1` doubles.
#[no_mangle]
pub unsafe extern "C" fn brainflow_get_psd(
    data: *const c_double,
    data_len: c_int,
    sampling_rate: c_double,
    window_type: c_int,
    psd_out: *mut c_double,
    freq_out: *mut c_double,
) -> c_int {
    if data.is_null() || data_len <= 0 {
        return INVALID_ARGUMENTS_ERROR;
    }

    let data_slice = slice::from_raw_parts(data, data_len as usize);
    let window = window_from_int(window_type);

    let (psd, freq) = brainflow_fft::psd::get_psd(data_slice, sampling_rate, window);

    if !psd_out.is_null() {
        ptr::copy_nonoverlapping(psd.as_ptr(), psd_out, psd.len());
    }
    if !freq_out.is_null() {
        ptr::copy_nonoverlapping(freq.as_ptr(), freq_out, freq.len());
    }

    STATUS_OK
}

/// Get band power from PSD.
#[no_mangle]
pub unsafe extern "C" fn brainflow_get_band_power(
    psd: *const c_double,
    freq: *const c_double,
    data_len: c_int,
    freq_start: c_double,
    freq_end: c_double,
    power_out: *mut c_double,
) -> c_int {
    if psd.is_null() || freq.is_null() || data_len <= 0 {
        return INVALID_ARGUMENTS_ERROR;
    }

    let psd_slice = slice::from_raw_parts(psd, data_len as usize);
    let freq_slice = slice::from_raw_parts(freq, data_len as usize);

    let power = brainflow_fft::psd::get_band_power(psd_slice, freq_slice, freq_start, freq_end);

    if !power_out.is_null() {
        *power_out = power;
    }

    STATUS_OK
}

/// Perform lowpass filter.
///
/// # Safety
/// `data` must point to `data_len` doubles. Data is modified in place.
#[no_mangle]
pub unsafe extern "C" fn brainflow_perform_lowpass(
    data: *mut c_double,
    data_len: c_int,
    sampling_rate: c_double,
    cutoff: c_double,
    order: c_int,
    filter_type: c_int,
    ripple: c_double,
) -> c_int {
    if data.is_null() || data_len <= 0 {
        return INVALID_ARGUMENTS_ERROR;
    }

    let data_slice = slice::from_raw_parts_mut(data, data_len as usize);
    let ftype = filter_type_from_int(filter_type);

    brainflow_filter::perform_lowpass(
        data_slice,
        sampling_rate,
        cutoff,
        order as usize,
        ftype,
        ripple,
    );

    STATUS_OK
}

/// Perform highpass filter.
#[no_mangle]
pub unsafe extern "C" fn brainflow_perform_highpass(
    data: *mut c_double,
    data_len: c_int,
    sampling_rate: c_double,
    cutoff: c_double,
    order: c_int,
    filter_type: c_int,
    ripple: c_double,
) -> c_int {
    if data.is_null() || data_len <= 0 {
        return INVALID_ARGUMENTS_ERROR;
    }

    let data_slice = slice::from_raw_parts_mut(data, data_len as usize);
    let ftype = filter_type_from_int(filter_type);

    brainflow_filter::perform_highpass(
        data_slice,
        sampling_rate,
        cutoff,
        order as usize,
        ftype,
        ripple,
    );

    STATUS_OK
}

/// Perform bandpass filter.
#[no_mangle]
pub unsafe extern "C" fn brainflow_perform_bandpass(
    data: *mut c_double,
    data_len: c_int,
    sampling_rate: c_double,
    low_cutoff: c_double,
    high_cutoff: c_double,
    order: c_int,
    filter_type: c_int,
    ripple: c_double,
) -> c_int {
    if data.is_null() || data_len <= 0 {
        return INVALID_ARGUMENTS_ERROR;
    }

    let data_slice = slice::from_raw_parts_mut(data, data_len as usize);
    let ftype = filter_type_from_int(filter_type);

    brainflow_filter::perform_bandpass(
        data_slice,
        sampling_rate,
        low_cutoff,
        high_cutoff,
        order as usize,
        ftype,
        ripple,
    );

    STATUS_OK
}

/// Perform bandstop (notch) filter.
#[no_mangle]
pub unsafe extern "C" fn brainflow_perform_bandstop(
    data: *mut c_double,
    data_len: c_int,
    sampling_rate: c_double,
    low_cutoff: c_double,
    high_cutoff: c_double,
    order: c_int,
    filter_type: c_int,
    ripple: c_double,
) -> c_int {
    if data.is_null() || data_len <= 0 {
        return INVALID_ARGUMENTS_ERROR;
    }

    let data_slice = slice::from_raw_parts_mut(data, data_len as usize);
    let ftype = filter_type_from_int(filter_type);

    brainflow_filter::perform_bandstop(
        data_slice,
        sampling_rate,
        low_cutoff,
        high_cutoff,
        order as usize,
        ftype,
        ripple,
    );

    STATUS_OK
}

/// Remove environmental noise (50/60 Hz).
#[no_mangle]
pub unsafe extern "C" fn brainflow_remove_environmental_noise(
    data: *mut c_double,
    data_len: c_int,
    sampling_rate: c_double,
    noise_type: c_int,
) -> c_int {
    if data.is_null() || data_len <= 0 {
        return INVALID_ARGUMENTS_ERROR;
    }

    let data_slice = slice::from_raw_parts_mut(data, data_len as usize);
    let ntype = noise_type_from_int(noise_type);

    brainflow_filter::remove_environmental_noise(data_slice, sampling_rate, ntype);

    STATUS_OK
}

/// Detrend data.
#[no_mangle]
pub unsafe extern "C" fn brainflow_detrend(
    data: *mut c_double,
    data_len: c_int,
    detrend_type: c_int,
) -> c_int {
    if data.is_null() || data_len <= 0 {
        return INVALID_ARGUMENTS_ERROR;
    }

    let data_slice = slice::from_raw_parts_mut(data, data_len as usize);
    let dtype = detrend_type_from_int(detrend_type);

    brainflow_filter::detrend(data_slice, dtype);

    STATUS_OK
}

/// Perform wavelet transform.
#[no_mangle]
pub unsafe extern "C" fn brainflow_perform_wavelet_transform(
    data: *const c_double,
    data_len: c_int,
    wavelet_type: c_int,
    decomposition_level: c_int,
    coeffs_out: *mut c_double,
    lengths_out: *mut c_int,
) -> c_int {
    if data.is_null() || data_len <= 0 {
        return INVALID_ARGUMENTS_ERROR;
    }

    let data_slice = slice::from_raw_parts(data, data_len as usize);
    let wavelet = wavelet_from_int(wavelet_type);

    let (coeffs, lengths) = brainflow_wavelet::perform_wavelet_transform(
        data_slice,
        wavelet,
        decomposition_level as usize,
    );

    if !coeffs_out.is_null() {
        ptr::copy_nonoverlapping(coeffs.as_ptr(), coeffs_out, coeffs.len());
    }
    if !lengths_out.is_null() {
        for (i, &len) in lengths.iter().enumerate() {
            *lengths_out.add(i) = len as c_int;
        }
    }

    STATUS_OK
}

/// Perform inverse wavelet transform.
///
/// # Safety
/// `coeffs` must point to `coeffs_len` doubles.
/// `lengths` must point to `levels + 2` ints (approximation + detail levels).
/// `data_out` must be large enough for the reconstructed signal.
#[no_mangle]
pub unsafe extern "C" fn brainflow_perform_inverse_wavelet_transform(
    coeffs: *const c_double,
    coeffs_len: c_int,
    lengths: *const c_int,
    num_lengths: c_int,
    wavelet_type: c_int,
    data_out: *mut c_double,
) -> c_int {
    if coeffs.is_null() || lengths.is_null() || coeffs_len <= 0 || num_lengths <= 0 {
        return INVALID_ARGUMENTS_ERROR;
    }

    let coeffs_slice = slice::from_raw_parts(coeffs, coeffs_len as usize);
    let lengths_slice = slice::from_raw_parts(lengths, num_lengths as usize);
    let lengths_vec: Vec<usize> = lengths_slice.iter().map(|&x| x as usize).collect();
    let wavelet = wavelet_from_int(wavelet_type);

    let result = brainflow_wavelet::perform_inverse_wavelet_transform(
        coeffs_slice,
        &lengths_vec,
        wavelet,
    );

    if !data_out.is_null() {
        ptr::copy_nonoverlapping(result.as_ptr(), data_out, result.len());
    }

    STATUS_OK
}

/// Perform wavelet denoising.
///
/// # Safety
/// `data` must point to `data_len` doubles. Data is modified in place.
///
/// # Arguments
/// * `denoise_method` - 0 = VisuShrink, 1 = SureShrink
/// * `threshold_type` - 0 = Soft, 1 = Hard
#[no_mangle]
pub unsafe extern "C" fn brainflow_perform_wavelet_denoising(
    data: *mut c_double,
    data_len: c_int,
    wavelet_type: c_int,
    decomposition_level: c_int,
    denoise_method: c_int,
    threshold_type: c_int,
) -> c_int {
    if data.is_null() || data_len <= 0 {
        return INVALID_ARGUMENTS_ERROR;
    }

    let data_slice = slice::from_raw_parts_mut(data, data_len as usize);
    let wavelet = wavelet_from_int(wavelet_type);
    let method = denoise_method_from_int(denoise_method);
    let threshold = threshold_type_from_int(threshold_type);

    brainflow_wavelet::perform_wavelet_denoising(
        data_slice,
        wavelet,
        decomposition_level as usize,
        method,
        threshold,
    );

    STATUS_OK
}

/// Downsample data.
#[no_mangle]
pub unsafe extern "C" fn brainflow_perform_downsampling(
    data: *const c_double,
    data_len: c_int,
    period: c_int,
    operation: c_int,
    output: *mut c_double,
) -> c_int {
    if data.is_null() || data_len <= 0 || period <= 0 {
        return INVALID_ARGUMENTS_ERROR;
    }

    let data_slice = slice::from_raw_parts(data, data_len as usize);
    let period = period as usize;

    // Operation: 0 = each, 1 = mean, 2 = median
    let result: Vec<f64> = match operation {
        1 => {
            // Mean
            data_slice
                .chunks(period)
                .map(|chunk| brainflow_math::stats::mean(chunk))
                .collect()
        }
        2 => {
            // Median
            data_slice
                .chunks(period)
                .map(|chunk| brainflow_math::stats::median(chunk))
                .collect()
        }
        _ => {
            // Each (take first)
            data_slice.iter().step_by(period).copied().collect()
        }
    };

    if !output.is_null() {
        ptr::copy_nonoverlapping(result.as_ptr(), output, result.len());
    }

    STATUS_OK
}

// ============================================================================
// Helper functions
// ============================================================================

fn board_id_from_int(id: c_int) -> Option<BoardId> {
    // Map integer to BoardId
    match id {
        -3 => Some(BoardId::PlaybackFile),
        -2 => Some(BoardId::Streaming),
        -1 => Some(BoardId::Synthetic),
        0 => Some(BoardId::Cyton),
        1 => Some(BoardId::Ganglion),
        2 => Some(BoardId::CytonDaisy),
        3 => Some(BoardId::Galea),
        4 => Some(BoardId::GanglionWifi),
        5 => Some(BoardId::CytonWifi),
        6 => Some(BoardId::CytonDaisyWifi),
        7 => Some(BoardId::BrainBit),
        8 => Some(BoardId::Unicorn),
        9 => Some(BoardId::CallibriEeg),
        10 => Some(BoardId::CallibriEmg),
        11 => Some(BoardId::CallibriEcg),
        12 => Some(BoardId::Fascia),
        13 => Some(BoardId::Notion1),
        14 => Some(BoardId::Notion2),
        15 => Some(BoardId::Ironbci),
        16 => Some(BoardId::GforcePro),
        17 => Some(BoardId::FreeEeg32),
        18 => Some(BoardId::BrainBitBled),
        19 => Some(BoardId::GforceDual),
        20 => Some(BoardId::GaleaSerial),
        21 => Some(BoardId::MuseSBled),
        22 => Some(BoardId::Muse2Bled),
        23 => Some(BoardId::Crown),
        24 => Some(BoardId::AntNeuroEe410),
        25 => Some(BoardId::AntNeuroEe411),
        26 => Some(BoardId::AntNeuroEe430),
        27 => Some(BoardId::AntNeuroEe211),
        28 => Some(BoardId::AntNeuroEe212),
        29 => Some(BoardId::AntNeuroEe213),
        30 => Some(BoardId::AntNeuroEe214),
        31 => Some(BoardId::AntNeuroEe215),
        32 => Some(BoardId::AntNeuroEe221),
        33 => Some(BoardId::AntNeuroEe222),
        34 => Some(BoardId::AntNeuroEe223),
        35 => Some(BoardId::AntNeuroEe224),
        36 => Some(BoardId::AntNeuroEe225),
        37 => Some(BoardId::Enophone),
        38 => Some(BoardId::Muse2),
        39 => Some(BoardId::MuseS),
        40 => Some(BoardId::BrainAlive),
        41 => Some(BoardId::Muse2016),
        42 => Some(BoardId::Muse2016Bled),
        43 => Some(BoardId::Pieeg),
        44 => Some(BoardId::Explore4Chan),
        45 => Some(BoardId::Explore8Chan),
        46 => Some(BoardId::GanglionNative),
        47 => Some(BoardId::Emotibit),
        48 => Some(BoardId::GaleaV4),
        49 => Some(BoardId::GaleaV4Serial),
        50 => Some(BoardId::NtlWifi),
        51 => Some(BoardId::AntNeuroEe511),
        52 => Some(BoardId::FreeEeg128),
        53 => Some(BoardId::AavaaV3),
        54 => Some(BoardId::Explore32Chan),
        55 => Some(BoardId::ExplorePlus8Chan),
        56 => Some(BoardId::ExplorePlus32Chan),
        57 => Some(BoardId::NeuropawnKnight),
        58 => Some(BoardId::Synchroni),
        _ => None,
    }
}

fn preset_from_int(preset: c_int) -> Preset {
    match preset {
        1 => Preset::Auxiliary,
        2 => Preset::Ancillary,
        _ => Preset::Default,
    }
}

fn window_from_int(window: c_int) -> WindowType {
    match window {
        1 => WindowType::Hanning,
        2 => WindowType::Hamming,
        3 => WindowType::BlackmanHarris,
        _ => WindowType::None,
    }
}

fn filter_type_from_int(ftype: c_int) -> FilterType {
    match ftype {
        1 => FilterType::ChebyshevI,
        2 => FilterType::Bessel,
        _ => FilterType::Butterworth,
    }
}

fn noise_type_from_int(ntype: c_int) -> NoiseType {
    match ntype {
        1 => NoiseType::Sixty,
        2 => NoiseType::FiftyAndSixty,
        _ => NoiseType::Fifty,
    }
}

fn detrend_type_from_int(dtype: c_int) -> DetrendType {
    match dtype {
        1 => DetrendType::Constant,
        2 => DetrendType::Linear,
        _ => DetrendType::None,
    }
}

fn wavelet_from_int(wtype: c_int) -> Wavelet {
    match wtype {
        1 => Wavelet::Daubechies(2),
        2 => Wavelet::Daubechies(3),
        3 => Wavelet::Daubechies(4),
        4 => Wavelet::Daubechies(5),
        5 => Wavelet::Daubechies(6),
        6 => Wavelet::Daubechies(7),
        7 => Wavelet::Daubechies(8),
        8 => Wavelet::Daubechies(9),
        9 => Wavelet::Daubechies(10),
        10 => Wavelet::Symlet(2),
        11 => Wavelet::Symlet(3),
        12 => Wavelet::Coiflet(1),
        _ => Wavelet::Haar,
    }
}

fn denoise_method_from_int(method: c_int) -> DenoiseMethod {
    match method {
        1 => DenoiseMethod::SureShrink,
        _ => DenoiseMethod::VisuShrink,
    }
}

fn threshold_type_from_int(ttype: c_int) -> ThresholdType {
    match ttype {
        1 => ThresholdType::Hard,
        _ => ThresholdType::Soft,
    }
}

unsafe fn parse_params_json(json: *const c_char) -> Result<BoardParams, String> {
    let c_str = CStr::from_ptr(json);
    let json_str = c_str.to_str().map_err(|_| "Invalid UTF-8")?;

    // Simple JSON parsing for board params
    let mut params = BoardParams::new();

    // Parse serial_port
    if let Some(start) = json_str.find("\"serial_port\"") {
        if let Some(value) = extract_string_value(&json_str[start..]) {
            params.serial_port = Some(value);
        }
    }

    // Parse ip_address
    if let Some(start) = json_str.find("\"ip_address\"") {
        if let Some(value) = extract_string_value(&json_str[start..]) {
            params.ip_address = Some(value);
        }
    }

    // Parse ip_port
    if let Some(start) = json_str.find("\"ip_port\"") {
        if let Some(value) = extract_int_value(&json_str[start..]) {
            params.ip_port = Some(value as u16);
        }
    }

    // Parse mac_address
    if let Some(start) = json_str.find("\"mac_address\"") {
        if let Some(value) = extract_string_value(&json_str[start..]) {
            params.mac_address = Some(value);
        }
    }

    // Parse file
    if let Some(start) = json_str.find("\"file\"") {
        if let Some(value) = extract_string_value(&json_str[start..]) {
            params.file = Some(value);
        }
    }

    Ok(params)
}

fn extract_string_value(s: &str) -> Option<String> {
    let colon = s.find(':')?;
    let rest = &s[colon + 1..];
    let start = rest.find('"')? + 1;
    let rest = &rest[start..];
    let end = rest.find('"')?;
    Some(rest[..end].to_string())
}

fn extract_int_value(s: &str) -> Option<i64> {
    let colon = s.find(':')?;
    let rest = s[colon + 1..].trim();
    let end = rest.find(|c: char| !c.is_ascii_digit() && c != '-').unwrap_or(rest.len());
    rest[..end].parse().ok()
}

fn error_to_code(e: &brainflow_sys::error::Error) -> c_int {
    use brainflow_sys::error::ErrorCode;
    match e.code {
        ErrorCode::GeneralError => GENERAL_ERROR,
        ErrorCode::InvalidArguments => INVALID_ARGUMENTS_ERROR,
        ErrorCode::BoardNotReady => BOARD_NOT_READY_ERROR,
        ErrorCode::StreamThreadNotRunning => STREAM_THREAD_NOT_RUNNING_ERROR,
        ErrorCode::NoDataInBuffer => EMPTY_BUFFER_ERROR,
        ErrorCode::BoardNotCreated => BOARD_NOT_CREATED_ERROR,
        ErrorCode::AnotherBoardCreated => ANOTHER_BOARD_IS_CREATED_ERROR,
        ErrorCode::UnsupportedBoard => UNSUPPORTED_BOARD_ERROR,
        _ => GENERAL_ERROR,
    }
}

// ============================================================================
// Version info
// ============================================================================

/// Get library version.
#[no_mangle]
pub extern "C" fn brainflow_get_version_string(
    version: *mut c_char,
    max_len: c_int,
) -> c_int {
    let version_str = "5.0.0-rust";

    if version.is_null() || max_len <= 0 {
        return INVALID_ARGUMENTS_ERROR;
    }

    unsafe {
        let bytes = version_str.as_bytes();
        let len = bytes.len().min((max_len - 1) as usize);
        ptr::copy_nonoverlapping(bytes.as_ptr(), version as *mut u8, len);
        *version.add(len) = 0; // null terminator
    }

    STATUS_OK
}

/// Set log level.
#[no_mangle]
pub extern "C" fn brainflow_set_log_level(_log_level: c_int) -> c_int {
    // TODO: Implement logging
    STATUS_OK
}

/// Enable/disable logging to file.
#[no_mangle]
pub extern "C" fn brainflow_set_log_file(_log_file: *const c_char) -> c_int {
    // TODO: Implement logging
    STATUS_OK
}
