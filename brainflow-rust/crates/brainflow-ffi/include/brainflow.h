/**
 * BrainFlow Rust Implementation - C FFI Header
 *
 * This header provides C-compatible function declarations for the BrainFlow
 * Rust library. It is designed to be ABI-compatible with the original C++
 * BrainFlow library.
 *
 * Usage:
 *   1. Link against libbrainflow_ffi.so (Linux), brainflow_ffi.dll (Windows),
 *      or libbrainflow_ffi.dylib (macOS)
 *   2. Include this header in your C/C++ code
 */

#ifndef BRAINFLOW_H
#define BRAINFLOW_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Error codes (matching C++ BrainFlow)
 * ============================================================================ */

#define BRAINFLOW_STATUS_OK                      0
#define BRAINFLOW_GENERAL_ERROR                  1
#define BRAINFLOW_INVALID_ARGUMENTS_ERROR        2
#define BRAINFLOW_BOARD_NOT_READY_ERROR          3
#define BRAINFLOW_STREAM_THREAD_NOT_RUNNING_ERROR 4
#define BRAINFLOW_EMPTY_BUFFER_ERROR             5
#define BRAINFLOW_BOARD_NOT_CREATED_ERROR        6
#define BRAINFLOW_ANOTHER_BOARD_IS_CREATED_ERROR 7
#define BRAINFLOW_UNSUPPORTED_BOARD_ERROR        8

/* ============================================================================
 * Board IDs
 * ============================================================================ */

#define BRAINFLOW_PLAYBACK_FILE_BOARD   -3
#define BRAINFLOW_STREAMING_BOARD       -2
#define BRAINFLOW_SYNTHETIC_BOARD       -1
#define BRAINFLOW_CYTON_BOARD            0
#define BRAINFLOW_GANGLION_BOARD         1
#define BRAINFLOW_CYTON_DAISY_BOARD      2
#define BRAINFLOW_GALEA_BOARD            3
#define BRAINFLOW_GANGLION_WIFI_BOARD    4
#define BRAINFLOW_CYTON_WIFI_BOARD       5
#define BRAINFLOW_CYTON_DAISY_WIFI_BOARD 6
#define BRAINFLOW_BRAINBIT_BOARD         7
#define BRAINFLOW_UNICORN_BOARD          8

/* ============================================================================
 * Preset types
 * ============================================================================ */

#define BRAINFLOW_PRESET_DEFAULT    0
#define BRAINFLOW_PRESET_AUXILIARY  1
#define BRAINFLOW_PRESET_ANCILLARY  2

/* ============================================================================
 * Window types (for FFT)
 * ============================================================================ */

#define BRAINFLOW_WINDOW_NONE           0
#define BRAINFLOW_WINDOW_HANNING        1
#define BRAINFLOW_WINDOW_HAMMING        2
#define BRAINFLOW_WINDOW_BLACKMAN_HARRIS 3

/* ============================================================================
 * Filter types
 * ============================================================================ */

#define BRAINFLOW_FILTER_BUTTERWORTH 0
#define BRAINFLOW_FILTER_CHEBYSHEV   1
#define BRAINFLOW_FILTER_BESSEL      2

/* ============================================================================
 * Noise types
 * ============================================================================ */

#define BRAINFLOW_NOISE_FIFTY       0
#define BRAINFLOW_NOISE_SIXTY       1
#define BRAINFLOW_NOISE_FIFTY_SIXTY 2

/* ============================================================================
 * Detrend types
 * ============================================================================ */

#define BRAINFLOW_DETREND_NONE     0
#define BRAINFLOW_DETREND_CONSTANT 1
#define BRAINFLOW_DETREND_LINEAR   2

/* ============================================================================
 * Wavelet types
 * ============================================================================ */

#define BRAINFLOW_WAVELET_HAAR   0
#define BRAINFLOW_WAVELET_DB2    1
#define BRAINFLOW_WAVELET_DB3    2
#define BRAINFLOW_WAVELET_DB4    3
#define BRAINFLOW_WAVELET_DB5    4
#define BRAINFLOW_WAVELET_DB6    5
#define BRAINFLOW_WAVELET_DB7    6
#define BRAINFLOW_WAVELET_DB8    7
#define BRAINFLOW_WAVELET_DB9    8
#define BRAINFLOW_WAVELET_DB10   9
#define BRAINFLOW_WAVELET_SYM2   10
#define BRAINFLOW_WAVELET_SYM3   11
#define BRAINFLOW_WAVELET_COIF1  12

/* ============================================================================
 * Denoise methods
 * ============================================================================ */

#define BRAINFLOW_DENOISE_VISUSHRINK 0
#define BRAINFLOW_DENOISE_SURESHRINK 1

/* ============================================================================
 * Threshold types
 * ============================================================================ */

#define BRAINFLOW_THRESHOLD_SOFT 0
#define BRAINFLOW_THRESHOLD_HARD 1

/* ============================================================================
 * Downsampling operations
 * ============================================================================ */

#define BRAINFLOW_DOWNSAMPLE_EACH   0
#define BRAINFLOW_DOWNSAMPLE_MEAN   1
#define BRAINFLOW_DOWNSAMPLE_MEDIAN 2

/* ============================================================================
 * Board control functions
 * ============================================================================ */

/**
 * Prepare a streaming session.
 *
 * @param board_id    Board ID (see BRAINFLOW_*_BOARD defines)
 * @param params_json JSON string with board parameters (can be NULL)
 * @return Error code (0 = success)
 */
int brainflow_prepare_session(int board_id, const char* params_json);

/**
 * Release a streaming session.
 *
 * @param board_id    Board ID
 * @param params_json JSON string with board parameters (can be NULL)
 * @return Error code
 */
int brainflow_release_session(int board_id, const char* params_json);

/**
 * Start data streaming.
 *
 * @param board_id     Board ID
 * @param buffer_size  Ring buffer size (use 0 for default)
 * @param params_json  JSON string with board parameters (can be NULL)
 * @return Error code
 */
int brainflow_start_stream(int board_id, int buffer_size, const char* params_json);

/**
 * Stop data streaming.
 *
 * @param board_id    Board ID
 * @param params_json JSON string with board parameters (can be NULL)
 * @return Error code
 */
int brainflow_stop_stream(int board_id, const char* params_json);

/**
 * Get current board data (peek, doesn't remove from buffer).
 *
 * @param board_id        Board ID
 * @param num_samples     Number of samples to get
 * @param preset          Preset type (0=default, 1=auxiliary, 2=ancillary)
 * @param params_json     JSON string with board parameters (can be NULL)
 * @param data_out        Output buffer (num_samples * num_channels doubles)
 * @param num_samples_out Actual number of samples retrieved
 * @return Error code
 */
int brainflow_get_current_board_data(
    int board_id,
    int num_samples,
    int preset,
    const char* params_json,
    double* data_out,
    int* num_samples_out
);

/**
 * Get board data (removes from buffer).
 *
 * @param board_id        Board ID
 * @param num_samples     Number of samples to get (0 for all available)
 * @param preset          Preset type
 * @param params_json     JSON string with board parameters (can be NULL)
 * @param data_out        Output buffer
 * @param num_samples_out Actual number of samples retrieved
 * @return Error code
 */
int brainflow_get_board_data(
    int board_id,
    int num_samples,
    int preset,
    const char* params_json,
    double* data_out,
    int* num_samples_out
);

/**
 * Get the number of samples in the buffer.
 *
 * @param board_id    Board ID
 * @param preset      Preset type
 * @param params_json JSON string with board parameters (can be NULL)
 * @param count_out   Output: number of samples available
 * @return Error code
 */
int brainflow_get_board_data_count(
    int board_id,
    int preset,
    const char* params_json,
    int* count_out
);

/**
 * Insert a marker into the data stream.
 *
 * @param board_id    Board ID
 * @param value       Marker value
 * @param preset      Preset type
 * @param params_json JSON string with board parameters (can be NULL)
 * @return Error code
 */
int brainflow_insert_marker(int board_id, double value, int preset, const char* params_json);

/**
 * Check if board is prepared.
 *
 * @param board_id    Board ID
 * @param params_json JSON string with board parameters (can be NULL)
 * @param is_prepared Output: 1 if prepared, 0 otherwise
 * @return Error code
 */
int brainflow_is_prepared(int board_id, const char* params_json, int* is_prepared);

/**
 * Get sampling rate for a board.
 *
 * @param board_id      Board ID
 * @param preset        Preset type
 * @param params_json   JSON string with board parameters (can be NULL)
 * @param sampling_rate Output: sampling rate in Hz
 * @return Error code
 */
int brainflow_get_sampling_rate(
    int board_id,
    int preset,
    const char* params_json,
    int* sampling_rate
);

/* ============================================================================
 * Signal processing functions
 * ============================================================================ */

/**
 * Perform FFT.
 *
 * @param data        Input data array
 * @param data_len    Length of input data
 * @param window_type Window function type
 * @param real_out    Output: real part of FFT (same length as input)
 * @param imag_out    Output: imaginary part of FFT
 * @return Error code
 */
int brainflow_perform_fft(
    const double* data,
    int data_len,
    int window_type,
    double* real_out,
    double* imag_out
);

/**
 * Perform inverse FFT.
 *
 * @param real     Real part of FFT coefficients
 * @param imag     Imaginary part of FFT coefficients
 * @param data_len Length of arrays
 * @param data_out Output: reconstructed signal
 * @return Error code
 */
int brainflow_perform_ifft(
    const double* real,
    const double* imag,
    int data_len,
    double* data_out
);

/**
 * Get power spectral density.
 *
 * @param data          Input data
 * @param data_len      Length of input data
 * @param sampling_rate Sampling rate in Hz
 * @param window_type   Window function type
 * @param psd_out       Output: PSD values (length = data_len/2 + 1)
 * @param freq_out      Output: frequency values
 * @return Error code
 */
int brainflow_get_psd(
    const double* data,
    int data_len,
    double sampling_rate,
    int window_type,
    double* psd_out,
    double* freq_out
);

/**
 * Get band power from PSD.
 *
 * @param psd        PSD values
 * @param freq       Frequency values
 * @param data_len   Length of arrays
 * @param freq_start Start frequency of band
 * @param freq_end   End frequency of band
 * @param power_out  Output: band power
 * @return Error code
 */
int brainflow_get_band_power(
    const double* psd,
    const double* freq,
    int data_len,
    double freq_start,
    double freq_end,
    double* power_out
);

/**
 * Perform lowpass filter.
 *
 * @param data          Data array (modified in place)
 * @param data_len      Length of data
 * @param sampling_rate Sampling rate in Hz
 * @param cutoff        Cutoff frequency
 * @param order         Filter order
 * @param filter_type   Filter type (0=Butterworth, 1=Chebyshev, 2=Bessel)
 * @param ripple        Ripple for Chebyshev filter (ignored for others)
 * @return Error code
 */
int brainflow_perform_lowpass(
    double* data,
    int data_len,
    double sampling_rate,
    double cutoff,
    int order,
    int filter_type,
    double ripple
);

/**
 * Perform highpass filter.
 */
int brainflow_perform_highpass(
    double* data,
    int data_len,
    double sampling_rate,
    double cutoff,
    int order,
    int filter_type,
    double ripple
);

/**
 * Perform bandpass filter.
 */
int brainflow_perform_bandpass(
    double* data,
    int data_len,
    double sampling_rate,
    double low_cutoff,
    double high_cutoff,
    int order,
    int filter_type,
    double ripple
);

/**
 * Perform bandstop (notch) filter.
 */
int brainflow_perform_bandstop(
    double* data,
    int data_len,
    double sampling_rate,
    double low_cutoff,
    double high_cutoff,
    int order,
    int filter_type,
    double ripple
);

/**
 * Remove environmental noise (50/60 Hz).
 *
 * @param data          Data array (modified in place)
 * @param data_len      Length of data
 * @param sampling_rate Sampling rate in Hz
 * @param noise_type    Noise type (0=50Hz, 1=60Hz, 2=both)
 * @return Error code
 */
int brainflow_remove_environmental_noise(
    double* data,
    int data_len,
    double sampling_rate,
    int noise_type
);

/**
 * Detrend data.
 *
 * @param data         Data array (modified in place)
 * @param data_len     Length of data
 * @param detrend_type Detrend type (0=none, 1=constant, 2=linear)
 * @return Error code
 */
int brainflow_detrend(double* data, int data_len, int detrend_type);

/**
 * Perform wavelet transform.
 *
 * @param data                Input data
 * @param data_len            Length of input data
 * @param wavelet_type        Wavelet type
 * @param decomposition_level Decomposition level
 * @param coeffs_out          Output: wavelet coefficients
 * @param lengths_out         Output: lengths of each level
 * @return Error code
 */
int brainflow_perform_wavelet_transform(
    const double* data,
    int data_len,
    int wavelet_type,
    int decomposition_level,
    double* coeffs_out,
    int* lengths_out
);

/**
 * Perform inverse wavelet transform.
 *
 * @param coeffs       Wavelet coefficients
 * @param coeffs_len   Total length of coefficients
 * @param lengths      Lengths of each level
 * @param num_lengths  Number of length entries
 * @param wavelet_type Wavelet type
 * @param data_out     Output: reconstructed signal
 * @return Error code
 */
int brainflow_perform_inverse_wavelet_transform(
    const double* coeffs,
    int coeffs_len,
    const int* lengths,
    int num_lengths,
    int wavelet_type,
    double* data_out
);

/**
 * Perform wavelet denoising.
 *
 * @param data                Data array (modified in place)
 * @param data_len            Length of data
 * @param wavelet_type        Wavelet type
 * @param decomposition_level Decomposition level
 * @param denoise_method      Denoise method (0=VisuShrink, 1=SureShrink)
 * @param threshold_type      Threshold type (0=Soft, 1=Hard)
 * @return Error code
 */
int brainflow_perform_wavelet_denoising(
    double* data,
    int data_len,
    int wavelet_type,
    int decomposition_level,
    int denoise_method,
    int threshold_type
);

/**
 * Downsample data.
 *
 * @param data      Input data
 * @param data_len  Length of input data
 * @param period    Downsampling period
 * @param operation Operation (0=each, 1=mean, 2=median)
 * @param output    Output: downsampled data (length = data_len / period)
 * @return Error code
 */
int brainflow_perform_downsampling(
    const double* data,
    int data_len,
    int period,
    int operation,
    double* output
);

/* ============================================================================
 * Version and logging
 * ============================================================================ */

/**
 * Get library version string.
 *
 * @param version Buffer to store version string
 * @param max_len Maximum length of buffer
 * @return Error code
 */
int brainflow_get_version_string(char* version, int max_len);

/**
 * Set log level.
 *
 * @param log_level Log level (0-6)
 * @return Error code
 */
int brainflow_set_log_level(int log_level);

/**
 * Enable logging to file.
 *
 * @param log_file Path to log file
 * @return Error code
 */
int brainflow_set_log_file(const char* log_file);

#ifdef __cplusplus
}
#endif

#endif /* BRAINFLOW_H */
