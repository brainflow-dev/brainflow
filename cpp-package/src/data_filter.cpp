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
    double *data, int data_len, char *wavelet, int decomposition_level)
{
    if (data_len <= 0)
    {
        throw BrainFlowException (
            "invalid input params", (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR);
    }

    double *wavelet_output = new double[data_len +
        2 * decomposition_level * (40 + 1)]; // I get this formula from wavelib sources
    int *decomposition_lengths = new int[decomposition_level + 1];
    int res = ::perform_wavelet_transform (
        data, data_len, wavelet, decomposition_level, wavelet_output, decomposition_lengths);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        delete[] wavelet_output;
        delete[] decomposition_lengths;
        throw BrainFlowException ("failed to perform wavelet", res);
    }
    return std::make_pair (wavelet_output, decomposition_lengths);
}

double *DataFilter::perform_inverse_wavelet_transform (std::pair<double *, int *> wavelet_output,
    int original_data_len, char *wavelet, int decomposition_level)
{
    if (original_data_len <= 0)
    {
        throw BrainFlowException (
            "invalid input params", (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR);
    }

    double *original_data = new double[original_data_len];
    int res = ::perform_inverse_wavelet_transform (wavelet_output.first, original_data_len, wavelet,
        decomposition_level, wavelet_output.second, original_data);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        delete[] original_data;
        throw BrainFlowException ("failed to perform inverse wavelet", res);
    }
    return original_data;
}

void DataFilter::perform_wavelet_denoising (
    double *data, int data_len, char *wavelet, int decomposition_level)
{
    int res = ::perform_wavelet_denoising (data, data_len, wavelet, decomposition_level);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to perform wavelet denoising", res);
    }
}

std::pair<double **, double *> DataFilter::get_csp (
    double ***data, double *labels, int n_epochs, int n_channels, int n_times)
{
    if ((data == NULL) || (labels == NULL))
    {
        throw BrainFlowException (
            "Invalid params", (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR);
    }

    double *temp_data1d = new double[n_epochs * n_channels * n_times];
    for (int e = 0; e < n_epochs; e++)
    {
        for (int c = 0; c < n_channels; c++)
        {
            for (int t = 0; t < n_times; t++)
            {
                int idx = e * n_channels * n_times + c * n_times + t;
                temp_data1d[idx] = data[e][c][t];
            }
        }
    }

    double *temp_filters = new double[n_channels * n_channels];
    double *output_eigenvalues = new double[n_channels];
    for (int i = 0; i < n_channels; i++)
    {
        output_eigenvalues[i] = 0;
        for (int j = 0; j < n_channels; j++)
        {
            temp_filters[i * n_channels + j] = 0;
        }
    }

    int res = ::get_csp (
        temp_data1d, labels, n_epochs, n_channels, n_times, temp_filters, output_eigenvalues);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        delete[] output_eigenvalues;
        delete[] temp_filters;
        delete[] temp_data1d;
        throw BrainFlowException ("failed to compute the CSP filters", res);
    }

    double **output_filters = new double *[n_channels];
    for (int i = 0; i < n_channels; i++)
    {
        output_filters[i] = new double[n_channels];
        for (int j = 0; j < n_channels; j++)
        {
            output_filters[i][j] = temp_filters[i * n_channels + j];
        }
    }

    delete[] temp_data1d;
    delete[] temp_filters;
    return std::make_pair (output_filters, output_eigenvalues);
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
    double **data, int cols, int *channels, int channels_len, int sampling_rate, bool apply_filters)
{
    if ((data == NULL) || (channels == NULL) || (channels_len < 1))
    {
        throw BrainFlowException (
            "Invalid params", (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR);
    }
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
            data_1d[j + cols * i] = data[channels[i]][j];
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

double **DataFilter::read_file (int *num_rows, int *num_cols, char *file_name)
{
    int max_elements = 0;
    int res = get_num_elements_in_file (file_name, &max_elements);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to determine file size", res);
    }
    double *data_linear = new double[max_elements];
    res = ::read_file (data_linear, num_rows, num_cols, file_name, max_elements);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        delete[] data_linear;
        throw BrainFlowException ("failed to read file", res);
    }
    double **output_buf = new double *[*num_rows];
    for (int i = 0; i < *num_rows; i++)
    {
        output_buf[i] = new double[*num_cols];
    }

    DataFilter::reshape_data_to_2d (*num_rows, *num_cols, data_linear, output_buf);
    delete[] data_linear;

    return output_buf;
}

void DataFilter::write_file (
    double **data, int num_rows, int num_cols, char *file_name, char *file_mode)
{
    double *data_linear = new double[num_rows * num_cols];
    DataFilter::reshape_data_to_1d (num_rows, num_cols, data, data_linear);
    int res = ::write_file (data_linear, num_rows, num_cols, file_name, file_mode);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        delete[] data_linear;
        throw BrainFlowException ("failed to write file", res);
    }
    delete[] data_linear;
}

void DataFilter::reshape_data_to_1d (int num_rows, int num_cols, double **buf, double *output_buf)
{
    for (int i = 0; i < num_cols; i++)
    {
        for (int j = 0; j < num_rows; j++)
        {
            output_buf[j * num_cols + i] = buf[j][i];
        }
    }
}

void DataFilter::reshape_data_to_2d (
    int num_rows, int num_cols, double *linear_buffer, double **output_buf)
{
    for (int i = 0; i < num_rows; i++)
    {
        for (int j = 0; j < num_cols; j++)
        {
            output_buf[i][j] = linear_buffer[i * num_cols + j];
        }
    }
}


/////////////////////////////////////////
//////////// logging methods ////////////
/////////////////////////////////////////

void DataFilter::set_log_level (int log_level)
{
    int res = ::set_log_level (log_level);
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

void DataFilter::set_log_file (char *log_file)
{
    int res = ::set_log_file (log_file);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to set log file", res);
    }
}