#include <cstdarg>
#include <stdlib.h>
#include <string.h>

#include "brainflow_constants.h"
#include "data_filter.h"
#include "data_handler.h"


double DataFilter::get_oxygen_level (double *ppg_ir, double *ppg_red, int data_len,
    int sampling_rate, double coef1, double coef2, double coef3)
{
    double value = 0.0;
    int res =
        ::get_oxygen_level (ppg_ir, ppg_red, data_len, sampling_rate, coef1, coef2, coef3, &value);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to calc spo2 value", res);
    }
    return value;
}

double DataFilter::get_heart_rate (
    double *ppg_ir, double *ppg_red, int data_len, int sampling_rate, int fft_size)
{
    double value = 0.0;
    int res = ::get_heart_rate (ppg_ir, ppg_red, data_len, sampling_rate, fft_size, &value);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to calc heart rate", res);
    }
    return value;
}

void DataFilter::perform_lowpass (double *data, int data_len, int sampling_rate, double cutoff,
    int order, int filter_type, double ripple)
{
    int res = ::perform_lowpass (data, data_len, sampling_rate, cutoff, order, filter_type, ripple);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to filter signal", res);
    }
}

void DataFilter::perform_highpass (double *data, int data_len, int sampling_rate, double cutoff,
    int order, int filter_type, double ripple)
{
    int res =
        ::perform_highpass (data, data_len, sampling_rate, cutoff, order, filter_type, ripple);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to filter signal", res);
    }
}

void DataFilter::perform_bandpass (double *data, int data_len, int sampling_rate, double start_freq,
    double stop_freq, int order, int filter_type, double ripple)
{
    int res = ::perform_bandpass (
        data, data_len, sampling_rate, start_freq, stop_freq, order, filter_type, ripple);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to filter signal", res);
    }
}

void DataFilter::perform_bandstop (double *data, int data_len, int sampling_rate, double start_freq,
    double stop_freq, int order, int filter_type, double ripple)
{
    int res = ::perform_bandstop (
        data, data_len, sampling_rate, start_freq, stop_freq, order, filter_type, ripple);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to filter signal", res);
    }
}

void DataFilter::remove_environmental_noise (
    double *data, int data_len, int sampling_rate, int noise_type)
{
    int res = ::remove_environmental_noise (data, data_len, sampling_rate, noise_type);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to remove environmental noise", res);
    }
}

void DataFilter::restore_data_from_wavelet_detailed_coeffs (double *data, int data_len, int wavelet,
    int decomposition_level, int level_to_restore, double *output)
{
    int res = ::restore_data_from_wavelet_detailed_coeffs (
        data, data_len, wavelet, decomposition_level, level_to_restore, output);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to restore", res);
    }
}

void DataFilter::detect_peaks_z_score (
    double *data, int data_len, int lag, double threshold, double influence, double *output)
{
    int res = ::detect_peaks_z_score (data, data_len, lag, threshold, influence, output);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to detect", res);
    }
}

void DataFilter::perform_rolling_filter (double *data, int data_len, int period, int agg_operation)
{
    int res = ::perform_rolling_filter (data, data_len, period, agg_operation);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to filter signal", res);
    }
}

double *DataFilter::perform_downsampling (
    double *data, int data_len, int period, int agg_operation, int *filtered_size)
{
    if ((data == NULL) || (data_len == 0) || (period == 0) || (data_len / period == 0))
    {
        throw BrainFlowException (
            "invalid input params", (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR);
    }
    double *filtered_data = new double[data_len / period];
    int res = ::perform_downsampling (data, data_len, period, agg_operation, filtered_data);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        delete[] filtered_data;
        throw BrainFlowException ("failed to filter signal", res);
    }
    *filtered_size = data_len / period;
    return filtered_data;
}

std::pair<double *, int *> DataFilter::perform_wavelet_transform (
    double *data, int data_len, int wavelet, int decomposition_level, int extension_type)
{
    if (data_len <= 0)
    {
        throw BrainFlowException (
            "invalid input params", (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR);
    }

    double *wavelet_output = new double[data_len +
        2 * decomposition_level * (40 + 1)]; // I get this formula from wavelib sources
    int *decomposition_lengths = new int[decomposition_level + 1];
    int res = ::perform_wavelet_transform (data, data_len, wavelet, decomposition_level,
        extension_type, wavelet_output, decomposition_lengths);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        delete[] wavelet_output;
        delete[] decomposition_lengths;
        throw BrainFlowException ("failed to perform wavelet", res);
    }
    return std::make_pair (wavelet_output, decomposition_lengths);
}

double *DataFilter::perform_inverse_wavelet_transform (std::pair<double *, int *> wavelet_output,
    int original_data_len, int wavelet, int decomposition_level, int extension_type)
{
    if (original_data_len <= 0)
    {
        throw BrainFlowException (
            "invalid input params", (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR);
    }

    double *original_data = new double[original_data_len];
    int res = ::perform_inverse_wavelet_transform (wavelet_output.first, original_data_len, wavelet,
        decomposition_level, extension_type, wavelet_output.second, original_data);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        delete[] original_data;
        throw BrainFlowException ("failed to perform inverse wavelet", res);
    }
    return original_data;
}

void DataFilter::perform_wavelet_denoising (double *data, int data_len, int wavelet,
    int decomposition_level, int wavelet_denoising, int threshold, int extenstion_type,
    int noise_level)
{
    int res = ::perform_wavelet_denoising (data, data_len, wavelet, decomposition_level,
        wavelet_denoising, threshold, extenstion_type, noise_level);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to perform wavelet denoising", res);
    }
}

std::pair<BrainFlowArray<double, 2>, BrainFlowArray<double, 1>> DataFilter::get_csp (
    const BrainFlowArray<double, 3> &data, const BrainFlowArray<double, 1> &labels)
{
    if ((data.empty ()) || (labels.empty ()))
    {
        throw BrainFlowException (
            "Invalid params", (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR);
    }

    BrainFlowArray<double, 2> filters (data.get_size (1), data.get_size (1));
    BrainFlowArray<double, 1> output_eigenvalues (data.get_size (1));

    int res =
        ::get_csp (data.get_raw_ptr (), labels.get_raw_ptr (), data.get_size (0), data.get_size (1),
            data.get_size (2), filters.get_raw_ptr (), output_eigenvalues.get_raw_ptr ());
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to compute the CSP filters", res);
    }

    return std::make_pair (std::move (filters), std::move (output_eigenvalues));
}

double *DataFilter::get_window (int window_function, int window_len)
{
    double *window_data = new double[window_len];
    int res = ::get_window (window_function, window_len, window_data);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        delete[] window_data;
        throw BrainFlowException ("failed to filter signal", res);
    }
    return window_data;
}

std::complex<double> *DataFilter::perform_fft (double *data, int data_len, int window, int *fft_len)
{
    if ((data_len % 2 == 1) || (data_len <= 0))
    {
        throw BrainFlowException (
            "data len must be even", (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR);
    }
    std::complex<double> *output = new std::complex<double>[data_len / 2 + 1];
    double *temp_re = new double[data_len / 2 + 1];
    double *temp_im = new double[data_len / 2 + 1];
    int res = ::perform_fft (data, data_len, window, temp_re, temp_im);
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        for (int i = 0; i < data_len / 2 + 1; i++)
        {
            output[i].real (temp_re[i]);
            output[i].imag (temp_im[i]);
        }
        delete[] temp_re;
        delete[] temp_im;
    }
    else
    {
        delete[] output;
        delete[] temp_re;
        delete[] temp_im;
        throw BrainFlowException ("failed to perform fft", res);
    }
    *fft_len = data_len / 2 + 1;
    return output;
}

void DataFilter::detrend (double *data, int data_len, int detrend_operation)
{
    int res = ::detrend (data, data_len, detrend_operation);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to detrend", res);
    }
}

std::pair<double *, double *> DataFilter::get_psd (
    double *data, int data_len, int sampling_rate, int window, int *psd_len)
{
    if ((data_len % 2 == 1) || (data_len <= 0))
    {
        throw BrainFlowException (
            "data len must be even", (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR);
    }
    double *ampl = new double[data_len / 2 + 1];
    double *freq = new double[data_len / 2 + 1];
    int res = ::get_psd (data, data_len, sampling_rate, window, ampl, freq);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        delete[] ampl;
        delete[] freq;
        throw BrainFlowException ("failed to get psd", res);
    }
    *psd_len = data_len / 2 + 1;
    return std::make_pair (ampl, freq);
}

std::pair<double *, double *> DataFilter::get_psd_welch (
    double *data, int data_len, int nfft, int overlap, int sampling_rate, int window, int *psd_len)
{
    if ((nfft % 2 == 1) || (data_len <= 0))
    {
        throw BrainFlowException (
            "nfft must be even", (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR);
    }
    double *ampl = new double[nfft / 2 + 1];
    double *freq = new double[nfft / 2 + 1];
    int res = ::get_psd_welch (data, data_len, nfft, overlap, sampling_rate, window, ampl, freq);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        delete[] ampl;
        delete[] freq;
        throw BrainFlowException ("failed to get_psd_welch", res);
    }
    *psd_len = nfft / 2 + 1;
    return std::make_pair (ampl, freq);
}

std::pair<double *, double *> DataFilter::get_avg_band_powers (
    const BrainFlowArray<double, 2> &data, std::vector<int> channels, int sampling_rate,
    bool apply_filters)
{
    std::vector<std::pair<double, double>> bands;
    bands.push_back (std::make_pair (2.0, 4.0));
    bands.push_back (std::make_pair (4.0, 8.0));
    bands.push_back (std::make_pair (8.0, 13.0));
    bands.push_back (std::make_pair (13.0, 30.0));
    bands.push_back (std::make_pair (30.0, 45.0));
    return get_custom_band_powers (data, bands, channels, sampling_rate, apply_filters);
}

std::pair<double *, double *> DataFilter::get_custom_band_powers (
    const BrainFlowArray<double, 2> &data, std::vector<std::pair<double, double>> bands,
    std::vector<int> channels, int sampling_rate, bool apply_filters)
{
    if ((data.empty ()) || (channels.empty ()) || (bands.empty ()))
    {
        throw BrainFlowException (
            "Invalid params", (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR);
    }
    int cols = data.get_size (1);
    int channels_len = (int)channels.size ();
    double *data_1d = new double[cols * channels_len];
    double *avg_bands = new double[bands.size ()];
    double *stddev_bands = new double[bands.size ()];
    double *start_freqs = new double[bands.size ()];
    double *stop_freqs = new double[bands.size ()];
    for (int i = 0; i < (int)bands.size (); i++)
    {
        start_freqs[i] = std::get<0> (bands[i]);
        stop_freqs[i] = std::get<1> (bands[i]);
        avg_bands[i] = 0.0;
        stddev_bands[i] = 0.0;
    }
    for (int i = 0; i < channels_len; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            data_1d[j + cols * i] = data.at (channels[i], j);
        }
    }
    int res = ::get_custom_band_powers (data_1d, channels_len, cols, start_freqs, stop_freqs,
        (int)bands.size (), sampling_rate, (int)apply_filters, avg_bands, stddev_bands);
    delete[] start_freqs;
    delete[] stop_freqs;
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        delete[] avg_bands;
        delete[] stddev_bands;
        delete[] data_1d;
        throw BrainFlowException ("failed to get_avg_band_powers", res);
    }
    delete[] data_1d;
    return std::make_pair (avg_bands, stddev_bands);
}

double DataFilter::get_band_power (
    std::pair<double *, double *> psd, int data_len, double freq_start, double freq_end)
{
    double band_power = 0;
    int res = ::get_band_power (psd.first, psd.second, data_len, freq_start, freq_end, &band_power);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to get band power", res);
    }
    return band_power;
}

double *DataFilter::perform_ifft (std::complex<double> *fft_data, int fft_len, int *data_len)
{
    if ((fft_len <= 0) || (fft_data == NULL) || (data_len == NULL))
    {
        throw BrainFlowException ("invalid args", (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR);
    }
    int original_size = (fft_len - 1) * 2;
    *data_len = original_size;
    double *output = new double[original_size];
    double *temp_re = new double[fft_len];
    double *temp_im = new double[fft_len];
    for (int i = 0; i < fft_len; i++)
    {
        temp_re[i] = fft_data[i].real ();
        temp_im[i] = fft_data[i].imag ();
    }

    int res = ::perform_ifft (temp_re, temp_im, original_size, output);
    delete[] temp_re;
    delete[] temp_im;
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        delete[] output;
        throw BrainFlowException ("failed to perform ifft", res);
    }
    return output;
}

int DataFilter::get_nearest_power_of_two (int value)
{
    int output = 0;
    int res = ::get_nearest_power_of_two (value, &output);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to calc nearest power of two", res);
    }
    return output;
}

BrainFlowArray<double, 2> DataFilter::read_file (std::string file_name)
{
    int max_elements = 0;
    int res = get_num_elements_in_file (file_name.c_str (), &max_elements);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to determine file size", res);
    }
    double *data_linear = new double[max_elements];
    memset (data_linear, 0, sizeof (double) * max_elements);
    int num_rows = 0;
    int num_cols = 0;
    res = ::read_file (data_linear, &num_rows, &num_cols, file_name.c_str (), max_elements);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        delete[] data_linear;
        throw BrainFlowException ("failed to read file", res);
    }
    BrainFlowArray<double, 2> data (data_linear, num_rows, num_cols);

    delete[] data_linear;

    return data;
}

void DataFilter::write_file (
    const BrainFlowArray<double, 2> &data, std::string file_name, std::string file_mode)
{
    int res = ::write_file (data.get_raw_ptr (), data.get_size (0), data.get_size (1),
        file_name.c_str (), file_mode.c_str ());
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to write file", res);
    }
}

/////////////////////////////////////////
//////////// logging methods ////////////
/////////////////////////////////////////

void DataFilter::set_log_level (int log_level)
{
    int res = set_log_level_data_handler (log_level);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to set log level", res);
    }
}

void DataFilter::enable_data_logger ()
{
    DataFilter::set_log_level ((int)LogLevels::LEVEL_INFO);
}

void DataFilter::disable_data_logger ()
{
    DataFilter::set_log_level ((int)LogLevels::LEVEL_OFF);
}

void DataFilter::enable_dev_data_logger ()
{
    DataFilter::set_log_level ((int)LogLevels::LEVEL_TRACE);
}

void DataFilter::set_log_file (std::string log_file)
{
    int res = set_log_file_data_handler (log_file.c_str ());
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to set log file", res);
    }
}

void DataFilter::log_message (int log_level, const char *format, ...)
{
    char buffer[1024];
    va_list ap;
    va_start (ap, format);
    vsnprintf (buffer, 1024, format, ap);
    va_end (ap);

    int res = log_message_data_handler (log_level, buffer);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to write log message", res);
    }
}

double DataFilter::calc_stddev (double *data, int start_pos, int end_pos)
{
    double output = 0;
    int res = ::calc_stddev (data, start_pos, end_pos, &output);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to calc stddev", res);
    }
    return output;
}

std::tuple<BrainFlowArray<double, 2>, BrainFlowArray<double, 2>, BrainFlowArray<double, 2>,
    BrainFlowArray<double, 2>>
DataFilter::perform_ica (
    const BrainFlowArray<double, 2> &data, int num_components, std::vector<int> channels)
{
    if ((data.empty ()) || (channels.empty ()) || (num_components < 1))
    {
        throw BrainFlowException (
            "Invalid params", (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR);
    }

    int cols = data.get_size (1);
    int channels_len = (int)channels.size ();
    double *data_1d = new double[cols * channels_len];
    double *w = new double[num_components * num_components];
    double *k = new double[channels_len * num_components];
    double *a = new double[num_components * channels_len];
    double *s = new double[cols * num_components];

    for (int i = 0; i < channels_len; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            data_1d[j + cols * i] = data.at (channels[i], j);
        }
    }
    int res = ::perform_ica (data_1d, channels_len, cols, num_components, w, k, a, s);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        delete[] data_1d;
        delete[] w;
        delete[] k;
        delete[] a;
        delete[] s;
        throw BrainFlowException ("failed to perform_ica", res);
    }
    BrainFlowArray<double, 2> w_mat (w, num_components, num_components);
    BrainFlowArray<double, 2> k_mat (k, num_components, channels_len);
    BrainFlowArray<double, 2> a_mat (a, channels_len, num_components);
    BrainFlowArray<double, 2> s_mat (s, num_components, cols);
    delete[] data_1d;
    delete[] w;
    delete[] k;
    delete[] a;
    delete[] s;
    return std::make_tuple (w_mat, k_mat, a_mat, s_mat);
}

std::tuple<BrainFlowArray<double, 2>, BrainFlowArray<double, 2>, BrainFlowArray<double, 2>,
    BrainFlowArray<double, 2>>
DataFilter::perform_ica (const BrainFlowArray<double, 2> &data, int num_components)
{
    std::vector<int> channels;
    int rows = data.get_size (0);
    for (int i = 0; i < rows; i++)
        channels.push_back (i);
    return perform_ica (data, num_components, channels);
}

double DataFilter::get_railed_percentage (double *data, int data_len, int gain)
{
    double output = 0;
    int res = ::get_railed_percentage (data, data_len, gain, &output);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to get railed percentege", res);
    }
    return output;
}

std::string DataFilter::get_version ()
{
    char version[64];
    int string_len = 0;
    int res = ::get_version_data_handler (version, &string_len, 64);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to get board info", res);
    }
    std::string verion_str (version, string_len);

    return verion_str;
}
