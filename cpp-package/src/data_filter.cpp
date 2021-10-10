#include <string.h>

#include "brainflow_constants.h"
#include "data_filter.h"
#include "data_handler.h"


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

void DataFilter::perform_bandpass (double *data, int data_len, int sampling_rate,
    double center_freq, double band_width, int order, int filter_type, double ripple)
{
    int res = ::perform_bandpass (
        data, data_len, sampling_rate, center_freq, band_width, order, filter_type, ripple);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to filter signal", res);
    }
}

void DataFilter::perform_bandstop (double *data, int data_len, int sampling_rate,
    double center_freq, double band_width, int order, int filter_type, double ripple)
{
    int res = ::perform_bandstop (
        data, data_len, sampling_rate, center_freq, band_width, order, filter_type, ripple);
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
    double *data, int data_len, std::string wavelet, int decomposition_level)
{
    if (data_len <= 0)
    {
        throw BrainFlowException (
            "invalid input params", (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR);
    }

    double *wavelet_output = new double[data_len +
        2 * decomposition_level * (40 + 1)]; // I get this formula from wavelib sources
    int *decomposition_lengths = new int[decomposition_level + 1];
    int res = ::perform_wavelet_transform (data, data_len, wavelet.c_str (), decomposition_level,
        wavelet_output, decomposition_lengths);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        delete[] wavelet_output;
        delete[] decomposition_lengths;
        throw BrainFlowException ("failed to perform wavelet", res);
    }
    return std::make_pair (wavelet_output, decomposition_lengths);
}

double *DataFilter::perform_inverse_wavelet_transform (std::pair<double *, int *> wavelet_output,
    int original_data_len, std::string wavelet, int decomposition_level)
{
    if (original_data_len <= 0)
    {
        throw BrainFlowException (
            "invalid input params", (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR);
    }

    double *original_data = new double[original_data_len];
    int res = ::perform_inverse_wavelet_transform (wavelet_output.first, original_data_len,
        wavelet.c_str (), decomposition_level, wavelet_output.second, original_data);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        delete[] original_data;
        throw BrainFlowException ("failed to perform inverse wavelet", res);
    }
    return original_data;
}

void DataFilter::perform_wavelet_denoising (
    double *data, int data_len, std::string wavelet, int decomposition_level)
{
    int res = ::perform_wavelet_denoising (data, data_len, wavelet.c_str (), decomposition_level);
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

std::complex<double> *DataFilter::perform_fft (double *data, int data_len, int window)
{
    if ((data_len & (data_len - 1)) || (data_len <= 0))
    {
        throw BrainFlowException (
            "data len is not power of 2", (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR);
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
    double *data, int data_len, int sampling_rate, int window)
{
    if ((data_len & (data_len - 1)) || (data_len <= 0))
    {
        throw BrainFlowException (
            "data len is not power of 2", (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR);
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
    return std::make_pair (ampl, freq);
}

std::pair<double *, double *> DataFilter::get_psd_welch (
    double *data, int data_len, int nfft, int overlap, int sampling_rate, int window)
{
    if ((nfft & (nfft - 1)) || (nfft <= 0))
    {
        throw BrainFlowException (
            "nfft is not power of 2", (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR);
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
    return std::make_pair (ampl, freq);
}

std::pair<double *, double *> DataFilter::get_avg_band_powers (
    const BrainFlowArray<double, 2> &data, std::vector<int> channels, int sampling_rate,
    bool apply_filters)
{
    if ((data.empty ()) || (channels.empty ()))
    {
        throw BrainFlowException (
            "Invalid params", (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR);
    }
    // todo refactor here and in low level api, no need to copy data
    int cols = data.get_size (1);
    int channels_len = (int)channels.size ();
    double *data_1d = new double[cols * channels_len];
    double *avg_bands = new double[5];
    double *stddev_bands = new double[5];
    // init by zeros to make valgrind happy
    for (int i = 0; i < 5; i++)
    {
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
    int res = ::get_avg_band_powers (
        data_1d, channels_len, cols, sampling_rate, (int)apply_filters, avg_bands, stddev_bands);
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

double *DataFilter::perform_ifft (std::complex<double> *data, int data_len)
{
    if ((data_len & (data_len - 1)) || (data_len <= 0))
    {
        throw BrainFlowException (
            "data len is not power of 2", (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR);
    }
    double *output = new double[data_len];
    double *temp_re = new double[data_len / 2 + 1];
    double *temp_im = new double[data_len / 2 + 1];
    for (int i = 0; i < data_len / 2 + 1; i++)
    {
        temp_re[i] = data[i].real ();
        temp_im[i] = data[i].imag ();
    }

    int res = ::perform_ifft (temp_re, temp_im, data_len, output);
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
