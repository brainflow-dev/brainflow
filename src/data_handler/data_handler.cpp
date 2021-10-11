#include <math.h>
#include <sstream>
#include <stdexcept>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <thread>
#include <vector>

#include "brainflow_constants.h"
#include "data_handler.h"
#include "downsample_operators.h"
#include "rolling_filter.h"
#include "wavelet_helpers.h"
#include "window_functions.h"

#include "DspFilters/Dsp.h"

#include "Eigen/Dense"

#include "wauxlib.h"
#include "wavelib.h"

#include "FFTReal.h"
#include "spdlog/sinks/null_sink.h"
#include "spdlog/spdlog.h"

#ifdef _OPENMP
#include <omp.h>
#endif

#define LOGGER_NAME "data_logger"
#define MAX_FILTER_ORDER 8

#ifdef __ANDROID__
#include "spdlog/sinks/android_sink.h"
std::shared_ptr<spdlog::logger> data_logger =
    spdlog::android_logger (LOGGER_NAME, "data_ndk_logger");
#else
std::shared_ptr<spdlog::logger> data_logger = spdlog::stderr_logger_mt (LOGGER_NAME);
#endif


int set_log_file_data_handler (const char *log_file)
{
#ifdef __ANDROID__
    data_logger->error ("For Android set_log_file is unavailable");
    return (int)BrainFlowExitCodes::GENERAL_ERROR;
#else
    try
    {
        spdlog::level::level_enum level = data_logger->level ();
        data_logger = spdlog::create<spdlog::sinks::null_sink_st> (
            "null_logger"); // to not set logger to nullptr and avoid race condition
        spdlog::drop (LOGGER_NAME);
        data_logger = spdlog::basic_logger_mt (LOGGER_NAME, log_file);
        data_logger->set_level (level);
        data_logger->flush_on (level);
        spdlog::drop ("null_logger");
    }
    catch (...)
    {
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
#endif
}

int set_log_level_data_handler (int level)
{
    int log_level = level;
    if (level > 6)
    {
        log_level = 6;
    }
    if (level < 0)
    {
        log_level = 0;
    }
    try
    {
        data_logger->set_level (spdlog::level::level_enum (log_level));
        data_logger->flush_on (spdlog::level::level_enum (log_level));
    }
    catch (...)
    {
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}


int perform_lowpass (double *data, int data_len, int sampling_rate, double cutoff, int order,
    int filter_type, double ripple)
{
    double *filter_data[1];
    filter_data[0] = data;
    Dsp::Filter *f = NULL;
    if ((order < 1) || (order > MAX_FILTER_ORDER) || (!data))
    {
        data_logger->error ("Order must be from 1-8 and data cannot be empty. Order:{} , Data:{}",
            order, (data != NULL));
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    switch (static_cast<FilterTypes> (filter_type))
    {
        case FilterTypes::BUTTERWORTH:
            f = new Dsp::FilterDesign<Dsp::Butterworth::Design::LowPass<MAX_FILTER_ORDER>, 1> ();
            break;
        case FilterTypes::CHEBYSHEV_TYPE_1:
            f = new Dsp::FilterDesign<Dsp::ChebyshevI::Design::LowPass<MAX_FILTER_ORDER>, 1> ();
            break;
        case FilterTypes::BESSEL:
            f = new Dsp::FilterDesign<Dsp::Bessel::Design::LowPass<MAX_FILTER_ORDER>, 1> ();
            break;
        default:
            data_logger->error ("Filter type {} is Invalid", filter_type);
            return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    Dsp::Params params;
    params[0] = sampling_rate; // sample rate
    params[1] = order;         // order
    params[2] = cutoff;        // cutoff
    if (filter_type == (int)FilterTypes::CHEBYSHEV_TYPE_1)
    {
        params[3] = ripple; // ripple
    }
    f->setParams (params);
    f->process (data_len, filter_data);
    delete f;

    return (int)BrainFlowExitCodes::STATUS_OK;
}

int perform_highpass (double *data, int data_len, int sampling_rate, double cutoff, int order,
    int filter_type, double ripple)
{
    Dsp::Filter *f = NULL;
    double *filter_data[1];
    filter_data[0] = data;

    if ((order < 1) || (order > MAX_FILTER_ORDER) || (!data))
    {
        data_logger->error ("Order must be from 1-8 and data cannot be empty. Order:{} , Data:{}",
            order, (data != NULL));
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    switch (static_cast<FilterTypes> (filter_type))
    {
        case FilterTypes::BUTTERWORTH:
            f = new Dsp::FilterDesign<Dsp::Butterworth::Design::HighPass<MAX_FILTER_ORDER>, 1> ();
            break;
        case FilterTypes::CHEBYSHEV_TYPE_1:
            f = new Dsp::FilterDesign<Dsp::ChebyshevI::Design::HighPass<MAX_FILTER_ORDER>, 1> ();
            break;
        case FilterTypes::BESSEL:
            f = new Dsp::FilterDesign<Dsp::Bessel::Design::HighPass<MAX_FILTER_ORDER>, 1> ();
            break;
        default:
            data_logger->error ("Filter type {} is Invalid", filter_type);
            return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    Dsp::Params params;
    params[0] = sampling_rate; // sample rate
    params[1] = order;         // order
    params[2] = cutoff;        // cutoff
    if (filter_type == (int)FilterTypes::CHEBYSHEV_TYPE_1)
    {
        params[3] = ripple; // ripple
    }
    f->setParams (params);
    f->process (data_len, filter_data);
    delete f;

    return (int)BrainFlowExitCodes::STATUS_OK;
}

int perform_bandpass (double *data, int data_len, int sampling_rate, double center_freq,
    double band_width, int order, int filter_type, double ripple)
{
    Dsp::Filter *f = NULL;
    double *filter_data[1];
    filter_data[0] = data;

    if ((order < 1) || (order > MAX_FILTER_ORDER) || (!data))
    {
        data_logger->error ("Order must be from 1-8 and data cannot be empty. Order:{} , Data:{}",
            order, (data != NULL));
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    switch (static_cast<FilterTypes> (filter_type))
    {
        case FilterTypes::BUTTERWORTH:
            f = new Dsp::FilterDesign<Dsp::Butterworth::Design::BandPass<MAX_FILTER_ORDER>, 1> ();
            break;
        case FilterTypes::CHEBYSHEV_TYPE_1:
            f = new Dsp::FilterDesign<Dsp::ChebyshevI::Design::BandPass<MAX_FILTER_ORDER>, 1> ();
            break;
        case FilterTypes::BESSEL:
            f = new Dsp::FilterDesign<Dsp::Bessel::Design::BandPass<MAX_FILTER_ORDER>, 1> ();
            break;
        default:
            data_logger->error ("Filter type {} is Invalid. ", filter_type);
            return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    Dsp::Params params;
    params[0] = sampling_rate; // sample rate
    params[1] = order;         // order
    params[2] = center_freq;   // center freq
    params[3] = band_width;
    if (filter_type == (int)FilterTypes::CHEBYSHEV_TYPE_1)
    {
        params[4] = ripple; // ripple
    }
    f->setParams (params);

    f->process (data_len, filter_data);
    delete f;

    return (int)BrainFlowExitCodes::STATUS_OK;
}

int perform_bandstop (double *data, int data_len, int sampling_rate, double center_freq,
    double band_width, int order, int filter_type, double ripple)
{
    Dsp::Filter *f = NULL;
    double *filter_data[1];
    filter_data[0] = data;

    if ((order < 1) || (order > MAX_FILTER_ORDER) || (!data))
    {
        data_logger->error ("Order must be from 1-8 and data cannot be empty. Order:{} , Data:{}",
            order, (data != NULL));
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    switch (static_cast<FilterTypes> (filter_type))
    {
        case FilterTypes::BUTTERWORTH:
            f = new Dsp::FilterDesign<Dsp::Butterworth::Design::BandStop<MAX_FILTER_ORDER>, 1> ();
            break;
        case FilterTypes::CHEBYSHEV_TYPE_1:
            f = new Dsp::FilterDesign<Dsp::ChebyshevI::Design::BandStop<MAX_FILTER_ORDER>, 1> ();
            break;
        case FilterTypes::BESSEL:
            f = new Dsp::FilterDesign<Dsp::Bessel::Design::BandStop<MAX_FILTER_ORDER>, 1> ();
            break;
        default:
            data_logger->error ("Filter type {} is Invalid", filter_type);
            return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    Dsp::Params params;
    params[0] = sampling_rate; // sample rate
    params[1] = order;         // order
    params[2] = center_freq;   // center freq
    params[3] = band_width;
    if (filter_type == (int)FilterTypes::CHEBYSHEV_TYPE_1)
    {
        params[4] = ripple; // ripple
    }
    f->setParams (params);
    f->process (data_len, filter_data);
    delete f;

    return (int)BrainFlowExitCodes::STATUS_OK;
}

int remove_environmental_noise (double *data, int data_len, int sampling_rate, int noise_type)
{
    if ((data_len < 1) || (sampling_rate < 1) || (!data))
    {
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    int res = (int)BrainFlowExitCodes::STATUS_OK;

    switch (static_cast<NoiseTypes> (noise_type))
    {
        case NoiseTypes::FIFTY:
            res = perform_bandstop (
                data, data_len, sampling_rate, 50.0, 4.0, 4, (int)FilterTypes::BUTTERWORTH, 0.0);
            break;
        case NoiseTypes::SIXTY:
            res = perform_bandstop (
                data, data_len, sampling_rate, 60.0, 4.0, 4, (int)FilterTypes::BUTTERWORTH, 0.0);
            break;
        default:
            data_logger->error ("Invalid noise type");
            return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    return res;
}

int perform_rolling_filter (double *data, int data_len, int period, int agg_operation)
{
    if ((data == NULL) || (period <= 0))
    {
        data_logger->error ("Period must be >= 0 and data cannot be empty. Data:{} , Period:{}",
            period, (data != NULL));
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    RollingFilter<double> *filter = NULL;
    switch (static_cast<AggOperations> (agg_operation))
    {
        case AggOperations::MEAN:
            filter = new RollingAverage<double> (period);
            break;
        case AggOperations::MEDIAN:
            filter = new RollingMedian<double> (period);
            break;
        case AggOperations::EACH:
            return (int)BrainFlowExitCodes::STATUS_OK;
        default:
            data_logger->error ("Invalid aggregate opteration:{}", agg_operation);
            return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    for (int i = 0; i < data_len; i++)
    {
        filter->add_data (data[i]);
        data[i] = filter->get_value ();
    }
    delete filter;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int perform_downsampling (
    double *data, int data_len, int period, int agg_operation, double *output_data)
{
    if ((data == NULL) || (data_len <= 0) || (period <= 0) || (output_data == NULL))
    {
        data_logger->error ("Period must be >= 0 and data and output_data cannot be NULL.");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    double (*downsampling_op) (double *, int);
    switch (static_cast<AggOperations> (agg_operation))
    {
        case AggOperations::MEAN:
            downsampling_op = downsample_mean;
            break;
        case AggOperations::MEDIAN:
            downsampling_op = downsample_median;
            break;
        case AggOperations::EACH:
            downsampling_op = downsample_each;
            break;
        default:
            data_logger->error (
                "Invalid aggregate opteration:{}. Must be mean,median, or each", agg_operation);
            return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    int num_values = data_len / period;
    for (int i = 0; i < num_values; i++)
    {
        output_data[i] = downsampling_op (data + i * period, period);
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

// https://github.com/rafat/wavelib/wiki/DWT-Example-Code
int perform_wavelet_transform (double *data, int data_len, const char *wavelet,
    int decomposition_level, double *output_data, int *decomposition_lengths)
{
    if ((data == NULL) || (data_len <= 0) || (wavelet == NULL) || (output_data == NULL) ||
        (!validate_wavelet (wavelet)) || (decomposition_lengths == NULL) ||
        (decomposition_level <= 0))
    {
        data_logger->error (
            "Please review arguments. Data/Output must  not be empty,and must provide a valid "
            "wavelet with decomposition arguments. Decomposition level must be > 0.");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    wave_object obj = NULL;
    wt_object wt = NULL;

    try
    {
        obj = wave_init (wavelet);
        wt = wt_init (obj, "dwt", data_len, decomposition_level);
        setDWTExtension (wt, "sym");
        setWTConv (wt, "direct");
        dwt (wt, data);
        for (int i = 0; i < wt->outlength; i++)
        {
            output_data[i] = wt->output[i];
        }
        for (int i = 0; i < decomposition_level + 1; i++)
        {
            decomposition_lengths[i] = wt->length[i];
        }
        wave_free (obj);
        wt_free (wt);
    }
    catch (...)
    {
        if (obj)
        {
            wave_free (obj);
        }
        if (wt)
        {
            wt_free (wt);
        }
        // more likely exception here occured because input buffer is to small to perform wavelet
        // transform
        data_logger->error ("Input buffer size issue(likely too small.");
        return (int)BrainFlowExitCodes::INVALID_BUFFER_SIZE_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

// inside wavelib inverse transform uses internal state from direct transform, dirty hack to restore
// it here
int perform_inverse_wavelet_transform (double *wavelet_coeffs, int original_data_len,
    const char *wavelet, int decomposition_level, int *decomposition_lengths, double *output_data)
{
    if ((wavelet_coeffs == NULL) || (decomposition_level <= 0) || (original_data_len <= 0) ||
        (wavelet == NULL) || (output_data == NULL) || (!validate_wavelet (wavelet)) ||
        (decomposition_lengths == NULL))
    {
        data_logger->error (
            "Please review arguments. Data/Output must  not be empty,and must provide a valid "
            "wavelet with decomposition arguments. Decomposition level must be > 0.");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    wave_object obj;
    wt_object wt;

    try
    {
        obj = wave_init (wavelet);
        wt = wt_init (obj, "dwt", original_data_len, decomposition_level);
        setDWTExtension (wt, "sym");
        setWTConv (wt, "direct");
        int total_len = 0;
        for (int i = 0; i < decomposition_level + 1; i++)
        {
            wt->length[i] = decomposition_lengths[i];
            total_len += decomposition_lengths[i];
        }
        for (int i = 0; i < total_len; i++)
        {
            wt->output[i] = wavelet_coeffs[i];
        }
        idwt (wt, output_data);
        wave_free (obj);
        wt_free (wt);
    }
    catch (...)
    {
        if (obj)
        {
            wave_free (obj);
        }
        if (wt)
        {
            wt_free (wt);
        }
        data_logger->error ("Input buffer size issue(likely too small.");
        // more likely exception here occured because input buffer is to small to perform wavelet
        // transform
        return (int)BrainFlowExitCodes::INVALID_BUFFER_SIZE_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int perform_wavelet_denoising (
    double *data, int data_len, const char *wavelet, int decomposition_level)
{
    if ((data == NULL) || (data_len <= 0) || (decomposition_level <= 0) ||
        (!validate_wavelet (wavelet)))
    {
        data_logger->error ("Please review arguments. Data must  not be empty,and must provide a "
                            "valid wavelet with decomposition arguments.");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    denoise_object obj;
    double *temp = new double[data_len];
    try
    {
        obj = denoise_init (data_len, decomposition_level, wavelet);
        setDenoiseMethod (obj, "visushrink");
        setDenoiseWTMethod (obj, "dwt");
        setDenoiseWTExtension (obj, "sym");
        setDenoiseParameters (obj, "soft", "all");
        denoise (obj, data, temp);
        for (int i = 0; i < data_len; i++)
        {
            data[i] = temp[i];
        }
        delete[] temp;
        temp = NULL;
        denoise_free (obj);
    }
    catch (...)
    {
        if (temp)
        {
            delete[] temp;
        }
        if (obj)
        {
            denoise_free (obj);
        }
        // more likely exception here occured because input buffer is to small to perform wavelet
        // transform
        data_logger->error ("Input buffer size issue(likely too small.");
        return (int)BrainFlowExitCodes::INVALID_BUFFER_SIZE_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int get_csp (const double *data, const double *labels, int n_epochs, int n_channels, int n_times,
    double *output_w, double *output_d)
{
    if ((!data) || (!labels) || n_epochs <= 0 || n_channels <= 0 || n_times <= 0)
    {
        data_logger->error ("Invalid function arguments provided. Please verify that all integer "
                            "arguments are positive and data and labels arrays aren't empty.");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    try
    {
        Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> sum1 (
            n_channels, n_channels);
        Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> sum2 (
            n_channels, n_channels);

        sum1.setZero ();
        sum2.setZero ();

        int n_class1 = 0;
        int n_class2 = 0;

        // Compute an averaged covariance matrix for each class
        for (int e = 0; e < n_epochs; e++)
        {
            Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> X (
                n_channels, n_times);
            for (int c = 0; c < n_channels; c++)
            {
                for (int t = 0; t < n_times; t++)
                {
                    X (c, t) = data[e * n_channels * n_times + c * n_times + t];
                }
            }

            // center data
            for (int i = 0; i < n_channels; i++)
            {
                double ctr = X.row (i).mean ();
                for (int j = 0; j < n_times; j++)
                {
                    X (i, j) -= ctr;
                }
            }

            // For centered data cov(X) = (X * X_T) / n
            switch (int (labels[e]))
            {
                case 0:
                    sum1 += ((X * X.transpose ()).eval ()) / double (n_times);
                    n_class1++;
                    break;
                case 1:
                    sum2 += ((X * X.transpose ()).eval ()) / double (n_times);
                    n_class2++;
                    break;
                default:
                    data_logger->error ("Invalid class label. Current class label: {}", labels[e]);
                    return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
                    break;
            }
        }

        sum1 /= double (n_class1);
        sum2 /= double (n_class2);

        // Compute the CSP filters
        Eigen::GeneralizedSelfAdjointEigenSolver<
            Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>
            ges (sum1, sum1 + sum2);

        for (int i = 0; i < n_channels; i++)
        {
            output_d[i] = ges.eigenvalues () (i);
            for (int j = 0; j < n_channels; j++)
            {
                output_w[i * n_channels + j] = ges.eigenvectors () (j, i);
            }
        }
    }
    catch (...)
    {
        data_logger->error ("Error with doing CSP filtering.");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int get_window (int window_function, int window_len, double *output_window)
{
    if ((window_len <= 0) || (window_function < 0) || (output_window == NULL))
    {
        data_logger->error ("Please check the arguments: data_len must be > 0, window_function >= "
                            "0 and output_window cannot be empty.");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    // from https://www.edn.com/windowing-functions-improve-fft-results-part-i/
    switch (static_cast<WindowFunctions> (window_function))
    {
        case WindowFunctions::NO_WINDOW:
            no_window_function (window_len, output_window);
            break;
        case WindowFunctions::HAMMING:
            hamming_function (window_len, output_window);
            break;
        case WindowFunctions::HANNING:
            hanning_function (window_len, output_window);
            break;
        case WindowFunctions::BLACKMAN_HARRIS:
            blackman_harris_function (window_len, output_window);
            break;
        default:
            data_logger->error ("Invalid Window function. Window function:{}", window_function);
            return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

/*
   FFTReal output | Positive FFT equiv.   | Negative FFT equiv.
   ---------------+-----------------------+-----------------------
   f [0]          | Real (bin 0)          | Real (bin 0)
   f [...]        | Real (bin ...)        | Real (bin ...)
   f [length/2]   | Real (bin length/2)   | Real (bin length/2)
   f [length/2+1] | Imag (bin 1)          | -Imag (bin 1)
   f [...]        | Imag (bin ...)        | -Imag (bin ...)
   f [length-1]   | Imag (bin length/2-1) | -Imag (bin length/2-1)

And FFT bins are distributed in f [] as above:

               |                | Positive FFT    | Negative FFT
   Bin         | Real part      | imaginary part  | imaginary part
   ------------+----------------+-----------------+---------------
   0           | f [0]          | 0               | 0
   1           | f [1]          | f [length/2+1]  | -f [length/2+1]
   ...         | f [...],       | f [...]         | -f [...]
   length/2-1  | f [length/2-1] | f [length-1]    | -f [length-1]
   length/2    | f [length/2]   | 0               | 0
   length/2+1  | f [length/2-1] | -f [length-1]   | f [length-1]
   ...         | f [...]        | -f [...]        | f [...]
   length-1    | f [1]          | -f [length/2+1] | f [length/2+1]
*/
int perform_fft (
    double *data, int data_len, int window_function, double *output_re, double *output_im)
{
    // must be power of 2
    if ((!data) || (!output_re) || (!output_im) || (data_len <= 0) || (data_len & (data_len - 1)))
    {
        data_logger->error ("Please check to make sure all arguments aren't empty and data_len is "
                            "a postive power of 2.");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    double *windowed_data = new double[data_len];
    get_window (window_function, data_len, windowed_data);
    for (int i = 0; i < data_len; i++)
    {
        windowed_data[i] *= data[i];
    }
    double *temp = new double[data_len];
    try
    {
        ffft::FFTReal<double> fft_object (data_len);
        fft_object.do_fft (temp, windowed_data);
        for (int i = 0; i < data_len / 2 + 1; i++)
        {
            output_re[i] = temp[i];
        }
        output_im[0] = 0.0;
        for (int count = 1, j = data_len / 2 + 1; j < data_len; j++, count++)
        {
            // add minus to make output exactly as in scipy
            output_im[count] = -temp[j];
        }
        output_im[data_len / 2] = 0.0;
        delete[] temp;
        delete[] windowed_data;
        windowed_data = NULL;
        temp = NULL;
    }
    catch (...)
    {
        if (temp)
        {
            delete[] temp;
        }
        if (windowed_data)
        {
            delete[] windowed_data;
        }
        data_logger->error ("Error with doing FFT processing.");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

// data_len here is an original size, not len of input_re input_im
int perform_ifft (double *input_re, double *input_im, int data_len, double *restored_data)
{
    // must be power of 2
    if ((!restored_data) || (!input_re) || (!input_im) || (data_len <= 0) ||
        (data_len & (data_len - 1)))
    {
        data_logger->error ("Please check to make sure all arguments aren't empty and data_len is "
                            "a postive power of 2.");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    double *temp = new double[data_len];
    try
    {
        ffft::FFTReal<double> fft_object (data_len);
        for (int i = 0; i < data_len / 2 + 1; i++)
        {
            temp[i] = input_re[i];
        }
        for (int count = 1, j = data_len / 2 + 1; j < data_len; j++, count++)
        {
            // add minus to make output exactly as in scipy
            temp[j] = -input_im[count];
        }
        fft_object.do_ifft (temp, restored_data);
        fft_object.rescale (restored_data);
        delete[] temp;
        temp = NULL;
    }
    catch (...)
    {
        if (temp)
        {
            delete[] temp;
        }
        data_logger->error ("Error with doing inverse FFT.");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int get_psd (double *data, int data_len, int sampling_rate, int window_function,
    double *output_ampl, double *output_freq)
{
    if ((data == NULL) || (sampling_rate < 1) || (data_len < 1) || (data_len & (data_len - 1)) ||
        (output_ampl == NULL) || (output_freq == NULL))
    {
        data_logger->error ("Please check to make sure all arguments aren't empty, sampling rate "
                            "is >=1 and data_len is a postive power of 2.");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    double *re = new double[data_len / 2 + 1];
    double *im = new double[data_len / 2 + 1];
    int res = perform_fft (data, data_len, window_function, re, im);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        delete[] re;
        delete[] im;
        return res;
    }
    double freq_res = (double)sampling_rate / (double)data_len;
    for (int i = 0; i < data_len / 2 + 1; i++)
    {
        // https://www.mathworks.com/help/signal/ug/power-spectral-density-estimates-using-fft.html
        output_ampl[i] = (re[i] * re[i] + im[i] * im[i]) / ((double)(sampling_rate * data_len));
        if ((i != 0) && (i != data_len / 2))
        {
            output_ampl[i] *= 2;
        }
        output_freq[i] = i * freq_res;
    }
    delete[] re;
    delete[] im;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int get_band_power (double *ampl, double *freq, int data_len, double freq_start, double freq_end,
    double *band_power)
{
    if ((ampl == NULL) || (freq == NULL) || (freq_start > freq_end) || (band_power == NULL) ||
        (data_len < 2))
    {
        data_logger->error ("Please check to make sure all arguments aren't empty, freq_start > "
                            "freq_end and data_len >=2");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    double res = 0.0;
    int counter = 0;
    double freq_res = freq[1] - freq[0];
    for (int i = 0; i < data_len - 1; i++)
    {
        if (freq[i] > freq_end)
        {
            break;
        }
        if (freq[i] >= freq_start)
        {
            res += 0.5 * freq_res * (ampl[i] + ampl[i + 1]);
            counter++;
        }
    }
    if (counter == 0)
    {
        data_logger->error ("No data between freq_end and freq_start.");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    *band_power = res;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int get_nearest_power_of_two (int value, int *output)
{
    if (value < 0)
    {
        data_logger->error ("Value must be postive. Value:{}", value);
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    if (value == 1)
    {
        *output = 2;
        return (int)BrainFlowExitCodes::STATUS_OK;
    }

    int32_t v = (int32_t)value;
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;            // next power of 2
    int x = v >> 1; // previous power of 2
    *output = (v - value) > (value - x) ? x : v;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int write_file (
    const double *data, int num_rows, int num_cols, const char *file_name, const char *file_mode)
{
    if ((strcmp (file_mode, "w") != 0) && (strcmp (file_mode, "w+") != 0) &&
        (strcmp (file_mode, "a") != 0) && (strcmp (file_mode, "a+") != 0))
    {
        data_logger->error ("Incorrect file_mode. File_mode:{}", file_mode);
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    FILE *fp;
    fp = fopen (file_name, file_mode);
    if (fp == NULL)
    {
        data_logger->error (
            "Couldn't open file with file_name and file_mode argument. File_Mode:{}, File_name:{}",
            file_mode, file_name);
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    // in read/write file data is transposed!
    for (int i = 0; i < num_cols; i++)
    {
        for (int j = 0; j < num_rows - 1; j++)
        {
            fprintf (fp, "%lf\t", data[j * num_cols + i]);
        }
        fprintf (fp, "%lf\n", data[(num_rows - 1) * num_cols + i]);
    }
    fclose (fp);
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int read_file (double *data, int *num_rows, int *num_cols, const char *file_name, int num_elements)
{
    if (num_elements <= 0)
    {
        data_logger->error ("Nummber or elements must be greater than 0.");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    FILE *fp;
    fp = fopen (file_name, "r");
    if (fp == NULL)
    {
        data_logger->error ("Couldn't read file {}", file_name);
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    char buf[4096];
    // rows and cols in tsv file, in data array its transposed!
    int total_rows = 0;
    int total_cols = 0;

    // count rows
    char c;
    for (c = getc (fp); !feof (fp); c = getc (fp))
    {
        if (c == '\n')
        {
            total_rows++;
        }
    }

    fseek (fp, 0, SEEK_SET);
    int current_row = 0;
    int cur_pos = 0;
    while (fgets (buf, sizeof (buf), fp) != NULL)
    {
        std::string tsv_string (buf);
        std::stringstream ss (tsv_string);
        std::vector<std::string> splitted;
        std::string tmp;
        char sep = '\t';
        if (tsv_string.find ('\t') == std::string::npos)
        {
            sep = ',';
        }
        while (std::getline (ss, tmp, sep))
        {
            if (tmp != "\n")
            {
                splitted.push_back (tmp);
            }
        }
        total_cols = (int)splitted.size ();
        for (int i = 0; i < total_cols; i++)
        {
            data[i * total_rows + current_row] = std::stod (splitted[i]);
            cur_pos++;
            if (cur_pos == (num_elements - 1))
            {
                *num_cols = current_row + 1;
                *num_rows = total_cols;
                fclose (fp);
                return (int)BrainFlowExitCodes::STATUS_OK;
            }
        }
        current_row++;
    }
    // more likely code below is unreachable
    *num_cols = total_rows;
    *num_rows = total_cols;
    fclose (fp);
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int get_num_elements_in_file (const char *file_name, int *num_elements)
{
    FILE *fp;
    fp = fopen (file_name, "r");
    if (fp == NULL)
    {
        data_logger->error ("Couldn't read file {}", file_name);
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    char buf[4096];
    int total_rows = 0;

    // count rows
    char c;
    for (c = getc (fp); !feof (fp); c = getc (fp))
    {
        if (c == '\n')
        {
            total_rows++;
        }
    }
    if (total_rows == 0)
    {
        *num_elements = 0;
        fclose (fp);
        data_logger->error ("Empty file {}", file_name);
        return (int)BrainFlowExitCodes::EMPTY_BUFFER_ERROR;
    }

    fseek (fp, 0, SEEK_SET);
    while (fgets (buf, sizeof (buf), fp) != NULL)
    {
        std::string tsv_string (buf);
        std::stringstream ss (tsv_string);
        std::vector<std::string> splitted;
        std::string tmp;
        char sep = '\t';
        if (tsv_string.find ('\t') == std::string::npos)
        {
            sep = ',';
        }
        while (std::getline (ss, tmp, sep))
        {
            if (tmp != "\n")
            {
                splitted.push_back (tmp);
            }
        }
        *num_elements = (int)splitted.size () * total_rows;
        fclose (fp);
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    *num_elements = 0;
    fclose (fp);
    data_logger->error ("File contents", file_name);
    return (int)BrainFlowExitCodes::EMPTY_BUFFER_ERROR;
}

int detrend (double *data, int data_len, int detrend_operation)
{
    if ((data == NULL) || (data_len < 1))
    {
        data_logger->error (
            "Incorrect Data arguments. Data must not be empty and data_len must be >=1");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    if (detrend_operation == (int)DetrendOperations::NONE)
    {
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    if (detrend_operation == (int)DetrendOperations::CONSTANT)
    {
        double mean = 0.0;
        // subtract mean from data
        for (int i = 0; i < data_len; i++)
        {
            mean += data[i];
        }
        mean /= data_len;
        for (int i = 0; i < data_len; i++)
        {
            data[i] -= mean;
        }
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    if (detrend_operation == (int)DetrendOperations::LINEAR)
    {
        // use mean and gradient to find a line
        double mean_x = data_len / 2.0;
        double mean_y = 0;
        for (int i = 0; i < data_len; i++)
        {
            mean_y += data[i];
        }
        mean_y /= data_len;
        double temp_xy = 0.0;
        double temp_xx = 0.0;
        for (int i = 0; i < data_len; i++)
        {
            temp_xy += i * data[i];
            temp_xx += i * i;
        }
        double s_xy = temp_xy / data_len - mean_x * mean_y;
        double s_xx = temp_xx / data_len - mean_x * mean_x;
        double grad = s_xy / s_xx;
        double y_int = mean_y - grad * mean_x;
        for (int i = 0; i < data_len; i++)
        {
            data[i] = data[i] - (grad * i + y_int);
        }
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    data_logger->error ("Detrend operation is incorrect. Detrend:{}", detrend_operation);
    return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
}

int get_psd_welch (double *data, int data_len, int nfft, int overlap, int sampling_rate,
    int window_function, double *output_ampl, double *output_freq)
{
    if ((data == NULL) || (data_len < 1) || (nfft & (nfft - 1)) || (output_ampl == NULL) ||
        (output_freq == NULL) || (sampling_rate < 1) || (overlap < 0) || (overlap > nfft))
    {
        data_logger->error ("Please review your arguments.");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    double *ampls = new double[nfft / 2 + 1];
    int counter = 0;
    for (int i = 0; i < nfft / 2 + 1; i++)
    {
        output_ampl[i] = 0.0;
    }
    for (int pos = 0; (pos + nfft) <= data_len; pos += (nfft - overlap), counter++)
    {
        int res = get_psd (data + pos, nfft, sampling_rate, window_function, ampls, output_freq);
        if (res != (int)BrainFlowExitCodes::STATUS_OK)
        {
            delete[] ampls;
            return res;
        }
        for (int i = 0; i < nfft / 2 + 1; i++)
        {
            output_ampl[i] += ampls[i];
        }
    }
    delete[] ampls;
    if (counter == 0)
    {
        data_logger->error ("Nfft must be less than data_len.");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    // average data
    for (int i = 0; i < nfft / 2; i++)
    {
        output_ampl[i] /= counter;
    }

    return (int)BrainFlowExitCodes::STATUS_OK;
}

int get_avg_band_powers (double *raw_data, int rows, int cols, int sampling_rate, int apply_filters,
    double *avg_band_powers, double *stddev_band_powers)
{
    if ((sampling_rate < 1) || (raw_data == NULL) || (rows < 1) || (cols < 1) ||
        (avg_band_powers == NULL) || (stddev_band_powers == NULL))
    {
        data_logger->error ("Please review your arguments.");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    // rows - channels, cols - datapoints
    int *exit_codes = new int[rows];
    for (int i = 0; i < rows; i++)
    {
        exit_codes[i] = (int)BrainFlowExitCodes::STATUS_OK;
    }
    int nfft = 0;
    get_nearest_power_of_two (sampling_rate, &nfft);
    nfft *= 2; // for resolution ~ 0.5
    // handle the case if nfft > number of data points
    // its valid case but results will not be accurate
    while (nfft > cols)
    {
        nfft /= 2;
    }
    if (nfft < 8)
    {
        data_logger->error ("Not enough data for calculation.");
        delete[] exit_codes;
        return (int)BrainFlowExitCodes::INVALID_BUFFER_SIZE_ERROR;
    }
    double **bands = new double *[5];
    for (int i = 0; i < 5; i++)
    {
        bands[i] = new double[rows];
        // to make valgrind happy
        for (int j = 0; j < rows; j++)
        {
            bands[i][j] = 0.0;
        }
    }

#pragma omp parallel for
    for (int i = 0; i < rows; i++)
    {
        double *ampls = new double[nfft / 2 + 1];
        double *freqs = new double[nfft / 2 + 1];
        double *thread_data = new double[cols];
        memcpy (thread_data, raw_data + i * cols, sizeof (double) * cols);

        if (apply_filters)
        {
            exit_codes[i] = detrend (thread_data, cols, (int)DetrendOperations::LINEAR);
            if (exit_codes[i] == (int)BrainFlowExitCodes::STATUS_OK)
            {
                exit_codes[i] = perform_bandstop (thread_data, cols, sampling_rate, 50.0, 4.0, 4,
                    (int)FilterTypes::BUTTERWORTH, 0.0);
            }
            if (exit_codes[i] == (int)BrainFlowExitCodes::STATUS_OK)
            {
                exit_codes[i] = perform_bandstop (thread_data, cols, sampling_rate, 60.0, 4.0, 4,
                    (int)FilterTypes::BUTTERWORTH, 0.0);
            }
            if (exit_codes[i] == (int)BrainFlowExitCodes::STATUS_OK)
            {
                exit_codes[i] = perform_bandpass (thread_data, cols, sampling_rate, 24.0, 47.0, 4,
                    (int)FilterTypes::BUTTERWORTH, 0.0);
            }
        }

        // use 80% overlap, as long as it works fast overlap param can be big
        exit_codes[i] = get_psd_welch (thread_data, cols, nfft, 4 * nfft / 5, sampling_rate,
            (int)WindowFunctions::HANNING, ampls, freqs);
        if (exit_codes[i] == (int)BrainFlowExitCodes::STATUS_OK)
        {
            exit_codes[i] = get_band_power (ampls, freqs, nfft / 2 + 1, 1.5, 4.0, &bands[0][i]);
        }
        if (exit_codes[i] == (int)BrainFlowExitCodes::STATUS_OK)
        {
            exit_codes[i] = get_band_power (ampls, freqs, nfft / 2 + 1, 4.0, 8.0, &bands[1][i]);
        }
        if (exit_codes[i] == (int)BrainFlowExitCodes::STATUS_OK)
        {
            exit_codes[i] = get_band_power (ampls, freqs, nfft / 2 + 1, 7.5, 13.0, &bands[2][i]);
        }
        if (exit_codes[i] == (int)BrainFlowExitCodes::STATUS_OK)
        {
            exit_codes[i] = get_band_power (ampls, freqs, nfft / 2 + 1, 13.0, 30.0, &bands[3][i]);
        }
        if (exit_codes[i] == (int)BrainFlowExitCodes::STATUS_OK)
        {
            exit_codes[i] = get_band_power (ampls, freqs, nfft / 2 + 1, 30.0, 45.0, &bands[4][i]);
        }

        delete[] ampls;
        delete[] freqs;
        delete[] thread_data;
    }

    for (int i = 0; i < rows; i++)
    {
        if (exit_codes[i] != (int)BrainFlowExitCodes::STATUS_OK)
        {
            int ec = exit_codes[i];
            delete[] exit_codes;
            for (int j = 0; j < 5; j++)
            {
                delete[] bands[j];
            }
            delete[] bands;
            return ec;
        }
    }

    // find average and stddev
    double avg_bands[5] = {0.0, 0.0, 0.0, 0.0, 0.0};
    double std_bands[5] = {0.0, 0.0, 0.0, 0.0, 0.0};
    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < rows; j++)
        {
            avg_bands[i] += bands[i][j];
        }
        avg_bands[i] /= rows;
        for (int j = 0; j < rows; j++)
        {
            std_bands[i] += (bands[i][j] - avg_bands[i]) * (bands[i][j] - avg_bands[i]);
        }
        std_bands[i] /= rows;
        std_bands[i] = sqrt (std_bands[i]);
    }
    // use relative band powers
    double sum = 0.0;
    for (int i = 0; i < 5; i++)
    {
        sum += avg_bands[i];
    }
    for (int i = 0; i < 5; i++)
    {
        avg_band_powers[i] = avg_bands[i] / sum;
        // use relative stddev to 'normalize'(doesnt ensure range between 0 and 1) it and keep
        // information about variance, division by max doesnt make any sense for stddev, it will
        // lose information about ratio between mean and deviation
        stddev_band_powers[i] = std_bands[i] / avg_bands[i];
    }

    delete[] exit_codes;
    for (int j = 0; j < 5; j++)
    {
        delete[] bands[j];
    }
    delete[] bands;

    return (int)BrainFlowExitCodes::STATUS_OK;
}
