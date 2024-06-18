#include <algorithm>
#include <math.h>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <thread>
#include <vector>

#include "brainflow_constants.h"
#include "brainflow_version.h"
#include "common_data_handler_helpers.h"
#include "data_handler.h"
#include "downsample_operators.h"
#include "rolling_filter.h"
#include "wavelet_helpers.h"
#include "window_functions.h"

#include "DspFilters/Dsp.h"

#include "Eigen/Dense"

#include "wauxlib.h"
#include "wavelib.h"

#include "kiss_fftr.h"

#include "spdlog/sinks/null_sink.h"
#include "spdlog/spdlog.h"

#include "fastica.h"

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

// its only for logging methods, other methods can be executed simultaneously
std::mutex data_mutex;


int log_message_data_handler (int log_level, char *log_message)
{
    // its a method for loggging from high level
    std::lock_guard<std::mutex> lock (data_mutex);
    if (log_level < 0)
    {
        data_logger->warn ("log level should be >= 0");
        log_level = 0;
    }
    else if (log_level > 6)
    {
        data_logger->warn ("log level should be <= 6");
        log_level = 6;
    }

    data_logger->log (spdlog::level::level_enum (log_level), "{}", log_message);

    return (int)BrainFlowExitCodes::STATUS_OK;
}

int set_log_file_data_handler (const char *log_file)
{
    std::lock_guard<std::mutex> lock (data_mutex);
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
    std::lock_guard<std::mutex> lock (data_mutex);
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
    if ((order < 1) || (order > MAX_FILTER_ORDER) || (!data) || (cutoff < 0) || (sampling_rate < 1))
    {
        data_logger->error (
            "Order must be from 1-8 and data cannot be empty. Order:{} , Data:{} , Cutoff:{}",
            order, (data != NULL), cutoff);
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    double *filter_data[1];
    filter_data[0] = data;
    Dsp::Filter *f = NULL;

    switch (static_cast<FilterTypes> (filter_type))
    {
        case FilterTypes::BUTTERWORTH:
            f = new Dsp::FilterDesign<Dsp::Butterworth::Design::LowPass<MAX_FILTER_ORDER>, 1> ();
            break;
        case FilterTypes::BUTTERWORTH_ZERO_PHASE:
            f = new Dsp::FilterDesign<Dsp::Butterworth::Design::LowPass<MAX_FILTER_ORDER>, 1> ();
            break;
        case FilterTypes::CHEBYSHEV_TYPE_1:
            f = new Dsp::FilterDesign<Dsp::ChebyshevI::Design::LowPass<MAX_FILTER_ORDER>, 1> ();
            break;
        case FilterTypes::CHEBYSHEV_TYPE_1_ZERO_PHASE:
            f = new Dsp::FilterDesign<Dsp::ChebyshevI::Design::LowPass<MAX_FILTER_ORDER>, 1> ();
            break;
        case FilterTypes::BESSEL:
            f = new Dsp::FilterDesign<Dsp::Bessel::Design::LowPass<MAX_FILTER_ORDER>, 1> ();
            break;
        case FilterTypes::BESSEL_ZERO_PHASE:
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
    if ((filter_type == (int)FilterTypes::CHEBYSHEV_TYPE_1) ||
        (filter_type == (int)FilterTypes::CHEBYSHEV_TYPE_1_ZERO_PHASE))
    {
        params[3] = ripple; // ripple
    }
    f->setParams (params);
    f->process (data_len, filter_data);
    if ((filter_type == (int)FilterTypes::BUTTERWORTH_ZERO_PHASE) ||
        (filter_type == (int)FilterTypes::CHEBYSHEV_TYPE_1_ZERO_PHASE) ||
        (filter_type == (int)FilterTypes::BESSEL_ZERO_PHASE))
    {
        reverse_array (data, data_len);
        f->process (data_len, filter_data);
        reverse_array (data, data_len);
    }
    delete f;

    return (int)BrainFlowExitCodes::STATUS_OK;
}

int perform_highpass (double *data, int data_len, int sampling_rate, double cutoff, int order,
    int filter_type, double ripple)
{
    if ((order < 1) || (order > MAX_FILTER_ORDER) || (!data) || (cutoff < 0) || (sampling_rate < 1))
    {
        data_logger->error (
            "Order must be from 1-8 and data cannot be empty. Order:{} , Data:{} , Cutoff:{}",
            order, (data != NULL), cutoff);
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    Dsp::Filter *f = NULL;
    double *filter_data[1];
    filter_data[0] = data;

    switch (static_cast<FilterTypes> (filter_type))
    {
        case FilterTypes::BUTTERWORTH:
            f = new Dsp::FilterDesign<Dsp::Butterworth::Design::HighPass<MAX_FILTER_ORDER>, 1> ();
            break;
        case FilterTypes::BUTTERWORTH_ZERO_PHASE:
            f = new Dsp::FilterDesign<Dsp::Butterworth::Design::HighPass<MAX_FILTER_ORDER>, 1> ();
            break;
        case FilterTypes::CHEBYSHEV_TYPE_1:
            f = new Dsp::FilterDesign<Dsp::ChebyshevI::Design::HighPass<MAX_FILTER_ORDER>, 1> ();
            break;
        case FilterTypes::CHEBYSHEV_TYPE_1_ZERO_PHASE:
            f = new Dsp::FilterDesign<Dsp::ChebyshevI::Design::HighPass<MAX_FILTER_ORDER>, 1> ();
            break;
        case FilterTypes::BESSEL:
            f = new Dsp::FilterDesign<Dsp::Bessel::Design::HighPass<MAX_FILTER_ORDER>, 1> ();
            break;
        case FilterTypes::BESSEL_ZERO_PHASE:
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
    if ((filter_type == (int)FilterTypes::CHEBYSHEV_TYPE_1) ||
        (filter_type == (int)FilterTypes::CHEBYSHEV_TYPE_1_ZERO_PHASE))
    {
        params[3] = ripple; // ripple
    }
    f->setParams (params);
    f->process (data_len, filter_data);
    if ((filter_type == (int)FilterTypes::BUTTERWORTH_ZERO_PHASE) ||
        (filter_type == (int)FilterTypes::CHEBYSHEV_TYPE_1_ZERO_PHASE) ||
        (filter_type == (int)FilterTypes::BESSEL_ZERO_PHASE))
    {
        reverse_array (data, data_len);
        f->process (data_len, filter_data);
        reverse_array (data, data_len);
    }
    delete f;

    return (int)BrainFlowExitCodes::STATUS_OK;
}

int perform_bandpass (double *data, int data_len, int sampling_rate, double start_freq,
    double stop_freq, int order, int filter_type, double ripple)
{
    if ((order < 1) || (order > MAX_FILTER_ORDER) || (!data) || (stop_freq <= start_freq) ||
        (start_freq < 0) || (sampling_rate < 1))
    {
        data_logger->error ("Order must be from 1-8 and data cannot be empty. Order:{} , Data:{} , "
                            "Start Freq:{} , Stop Freq:{}",
            order, (data != NULL), start_freq, stop_freq);
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    double center_freq = (start_freq + stop_freq) / 2.0;
    double band_width = stop_freq - start_freq;
    Dsp::Filter *f = NULL;
    double *filter_data[1];
    filter_data[0] = data;

    switch (static_cast<FilterTypes> (filter_type))
    {
        case FilterTypes::BUTTERWORTH:
            f = new Dsp::FilterDesign<Dsp::Butterworth::Design::BandPass<MAX_FILTER_ORDER>, 1> ();
            break;
        case FilterTypes::BUTTERWORTH_ZERO_PHASE:
            f = new Dsp::FilterDesign<Dsp::Butterworth::Design::BandPass<MAX_FILTER_ORDER>, 1> ();
            break;
        case FilterTypes::CHEBYSHEV_TYPE_1:
            f = new Dsp::FilterDesign<Dsp::ChebyshevI::Design::BandPass<MAX_FILTER_ORDER>, 1> ();
            break;
        case FilterTypes::CHEBYSHEV_TYPE_1_ZERO_PHASE:
            f = new Dsp::FilterDesign<Dsp::ChebyshevI::Design::BandPass<MAX_FILTER_ORDER>, 1> ();
            break;
        case FilterTypes::BESSEL:
            f = new Dsp::FilterDesign<Dsp::Bessel::Design::BandPass<MAX_FILTER_ORDER>, 1> ();
            break;
        case FilterTypes::BESSEL_ZERO_PHASE:
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
    if ((filter_type == (int)FilterTypes::CHEBYSHEV_TYPE_1) ||
        (filter_type == (int)FilterTypes::CHEBYSHEV_TYPE_1_ZERO_PHASE))
    {
        params[3] = ripple; // ripple
    }
    f->setParams (params);
    f->process (data_len, filter_data);
    if ((filter_type == (int)FilterTypes::BUTTERWORTH_ZERO_PHASE) ||
        (filter_type == (int)FilterTypes::CHEBYSHEV_TYPE_1_ZERO_PHASE) ||
        (filter_type == (int)FilterTypes::BESSEL_ZERO_PHASE))
    {
        reverse_array (data, data_len);
        f->process (data_len, filter_data);
        reverse_array (data, data_len);
    }
    delete f;

    return (int)BrainFlowExitCodes::STATUS_OK;
}

int perform_bandstop (double *data, int data_len, int sampling_rate, double start_freq,
    double stop_freq, int order, int filter_type, double ripple)
{
    if ((order < 1) || (order > MAX_FILTER_ORDER) || (!data) || (stop_freq <= start_freq) ||
        (start_freq < 0) || (sampling_rate < 1))
    {
        data_logger->error ("Order must be from 1-8 and data cannot be empty. Order:{} , Data:{} , "
                            "Start Freq:{} , Stop Freq:{}",
            order, (data != NULL), start_freq, stop_freq);
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    double center_freq = (start_freq + stop_freq) / 2.0;
    double band_width = stop_freq - start_freq;
    Dsp::Filter *f = NULL;
    double *filter_data[1];
    filter_data[0] = data;

    switch (static_cast<FilterTypes> (filter_type))
    {
        case FilterTypes::BUTTERWORTH:
            f = new Dsp::FilterDesign<Dsp::Butterworth::Design::BandStop<MAX_FILTER_ORDER>, 1> ();
            break;
        case FilterTypes::BUTTERWORTH_ZERO_PHASE:
            f = new Dsp::FilterDesign<Dsp::Butterworth::Design::BandStop<MAX_FILTER_ORDER>, 1> ();
            break;
        case FilterTypes::CHEBYSHEV_TYPE_1:
            f = new Dsp::FilterDesign<Dsp::ChebyshevI::Design::BandStop<MAX_FILTER_ORDER>, 1> ();
            break;
        case FilterTypes::CHEBYSHEV_TYPE_1_ZERO_PHASE:
            f = new Dsp::FilterDesign<Dsp::ChebyshevI::Design::BandStop<MAX_FILTER_ORDER>, 1> ();
            break;
        case FilterTypes::BESSEL:
            f = new Dsp::FilterDesign<Dsp::Bessel::Design::BandStop<MAX_FILTER_ORDER>, 1> ();
            break;
        case FilterTypes::BESSEL_ZERO_PHASE:
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
    if ((filter_type == (int)FilterTypes::CHEBYSHEV_TYPE_1) ||
        (filter_type == (int)FilterTypes::CHEBYSHEV_TYPE_1_ZERO_PHASE))
    {
        params[3] = ripple; // ripple
    }
    f->setParams (params);
    f->process (data_len, filter_data);
    if ((filter_type == (int)FilterTypes::BUTTERWORTH_ZERO_PHASE) ||
        (filter_type == (int)FilterTypes::CHEBYSHEV_TYPE_1_ZERO_PHASE) ||
        (filter_type == (int)FilterTypes::BESSEL_ZERO_PHASE))
    {
        reverse_array (data, data_len);
        f->process (data_len, filter_data);
        reverse_array (data, data_len);
    }
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
            res = perform_bandstop (data, data_len, sampling_rate, 48.0, 52.0, 4,
                (int)FilterTypes::BUTTERWORTH_ZERO_PHASE, 0.0);
            break;
        case NoiseTypes::SIXTY:
            res = perform_bandstop (data, data_len, sampling_rate, 58.0, 62.0, 4,
                (int)FilterTypes::BUTTERWORTH_ZERO_PHASE, 0.0);
            break;
        case NoiseTypes::FIFTY_AND_SIXTY:
            res = perform_bandstop (data, data_len, sampling_rate, 48.0, 52.0, 4,
                (int)FilterTypes::BUTTERWORTH_ZERO_PHASE, 0.0);
            if (res == (int)BrainFlowExitCodes::STATUS_OK)
            {
                res = perform_bandstop (data, data_len, sampling_rate, 58.0, 62.0, 4,
                    (int)FilterTypes::BUTTERWORTH, 0.0);
            }
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
int perform_wavelet_transform (double *data, int data_len, int wavelet, int decomposition_level,
    int extension, double *output_data, int *decomposition_lengths)
{
    std::string wavelet_str = get_wavelet_name (wavelet);
    std::string extension_str = get_extension_type (extension);
    if ((data == NULL) || (data_len <= 0) || (wavelet_str.empty ()) || (output_data == NULL) ||
        (extension_str.empty ()) || (decomposition_lengths == NULL) || (decomposition_level <= 0))
    {
        data_logger->error ("Please review arguments.");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    wave_object obj = NULL;
    wt_object wt = NULL;

    try
    {
        obj = wave_init (wavelet_str.c_str ());
        wt = wt_init (obj, "dwt", data_len, decomposition_level);
        setDWTExtension (wt, extension_str.c_str ());
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
        obj = NULL;
        wt_free (wt);
        wt = NULL;
    }
    catch (const std::exception &e)
    {
        if (obj)
        {
            wave_free (obj);
            obj = NULL;
        }
        if (wt)
        {
            wt_free (wt);
            wt = NULL;
        }
        // more likely exception here occured because input buffer is to small to perform wavelet
        // transform
        data_logger->error ("Exception in wavelib: {}", e.what ());
        return (int)BrainFlowExitCodes::INVALID_BUFFER_SIZE_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

// inside wavelib inverse transform uses internal state from direct transform, dirty hack to restore
// it here
int perform_inverse_wavelet_transform (double *wavelet_coeffs, int original_data_len, int wavelet,
    int decomposition_level, int extension, int *decomposition_lengths, double *output_data)
{
    std::string wavelet_str = get_wavelet_name (wavelet);
    std::string extension_str = get_extension_type (extension);
    if ((wavelet_coeffs == NULL) || (decomposition_level <= 0) || (original_data_len <= 0) ||
        (output_data == NULL) || (wavelet_str.empty ()) || (extension_str.empty ()) ||
        (decomposition_lengths == NULL))
    {
        data_logger->error ("Please review arguments.");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    wave_object obj = NULL;
    wt_object wt = NULL;

    try
    {
        obj = wave_init (wavelet_str.c_str ());
        wt = wt_init (obj, "dwt", original_data_len, decomposition_level);
        setDWTExtension (wt, extension_str.c_str ());
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
        obj = NULL;
        wt_free (wt);
        wt = NULL;
    }
    catch (const std::exception &e)
    {
        if (obj)
        {
            wave_free (obj);
            obj = NULL;
        }
        if (wt)
        {
            wt_free (wt);
            wt = NULL;
        }
        data_logger->error ("Exception in wavelib: {}", e.what ());
        // more likely exception here occured because input buffer is to small to perform wavelet
        // transform
        return (int)BrainFlowExitCodes::INVALID_BUFFER_SIZE_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int perform_wavelet_denoising (double *data, int data_len, int wavelet, int decomposition_level,
    int wavelet_denoising, int threshold, int extenstion_type, int noise_level)
{
    std::string wavelet_str = get_wavelet_name (wavelet);
    std::string denoising_str = get_wavelet_denoising_type (wavelet_denoising);
    std::string threshold_str = get_threshold_type (threshold);
    std::string extension_str = get_extension_type (extenstion_type);
    std::string noise_str = get_noise_estimation_type (noise_level);
    if ((data == NULL) || (data_len <= 0) || (decomposition_level <= 0) || (wavelet_str.empty ()) ||
        (denoising_str.empty ()) || (threshold_str.empty ()) || (extension_str.empty ()) ||
        (noise_str.empty ()))
    {
        data_logger->error ("Please review arguments.");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    denoise_object obj = NULL;
    double *temp = new double[data_len];
    try
    {
        obj = denoise_init (data_len, decomposition_level, wavelet_str.c_str ());
        setDenoiseMethod (obj, denoising_str.c_str ());
        setDenoiseWTMethod (obj, "dwt");
        setDenoiseWTExtension (obj, extension_str.c_str ());
        setDenoiseParameters (obj, threshold_str.c_str (), noise_str.c_str ());
        denoise (obj, data, temp);
        for (int i = 0; i < data_len; i++)
        {
            data[i] = temp[i];
        }
        delete[] temp;
        temp = NULL;
        denoise_free (obj);
        obj = NULL;
    }
    catch (const std::exception &e)
    {
        if (temp)
        {
            delete[] temp;
            temp = NULL;
        }
        if (obj)
        {
            denoise_free (obj);
            obj = NULL;
        }
        // more likely exception here occured because input buffer is to small to perform wavelet
        // transform
        data_logger->error ("Exception in wavelib: {}", e.what ());
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
    switch (static_cast<WindowOperations> (window_function))
    {
        case WindowOperations::NO_WINDOW:
            no_window_function (window_len, output_window);
            break;
        case WindowOperations::HAMMING:
            hamming_function (window_len, output_window);
            break;
        case WindowOperations::HANNING:
            hanning_function (window_len, output_window);
            break;
        case WindowOperations::BLACKMAN_HARRIS:
            blackman_harris_function (window_len, output_window);
            break;
        default:
            data_logger->error ("Invalid Window function. Window function:{}", window_function);
            return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int perform_fft (
    double *data, int data_len, int window_function, double *output_re, double *output_im)
{
    if ((!data) || (!output_re) || (!output_im) || (data_len <= 0) || (data_len % 2 == 1))
    {
        data_logger->error (
            "Please check to make sure all arguments aren't empty and data_len is even.");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    double *windowed_data = new double[data_len];
    get_window (window_function, data_len, windowed_data);
    for (int i = 0; i < data_len; i++)
    {
        windowed_data[i] *= data[i];
    }
    kiss_fft_cpx *sout = new kiss_fft_cpx[data_len];
    kiss_fftr_cfg cfg = NULL;
    try
    {
        cfg = kiss_fftr_alloc (data_len, 0, 0, 0);
        kiss_fftr (cfg, windowed_data, sout);
        for (int i = 0; i < data_len / 2 + 1; i++)
        {
            output_re[i] = sout[i].r;
            output_im[i] = sout[i].i;
        }
        delete[] sout;
        delete[] windowed_data;
        windowed_data = NULL;
        sout = NULL;
        kiss_fftr_free (cfg);
    }
    catch (...)
    {
        if (sout)
        {
            delete[] sout;
        }
        if (windowed_data)
        {
            delete[] windowed_data;
        }
        if (cfg)
        {
            kiss_fftr_free (cfg);
        }
        data_logger->error ("Error with doing FFT processing.");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

// data_len here is an original size, not len of input_re input_im
int perform_ifft (double *input_re, double *input_im, int data_len, double *restored_data)
{
    if ((!restored_data) || (!input_re) || (!input_im) || (data_len <= 0) || (data_len % 2 == 1))
    {
        data_logger->error (
            "Please check to make sure all arguments aren't empty and data_len is even.");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    double *temp = new double[data_len];
    kiss_fft_cpx *cin = new kiss_fft_cpx[data_len];
    for (int i = 0; i < data_len / 2 + 1; i++)
    {
        cin[i].r = input_re[i];
        cin[i].i = input_im[i];
    }
    kiss_fftr_cfg cfg = NULL;
    try
    {
        cfg = kiss_fftr_alloc (data_len, 1, 0, 0);
        kiss_fftri (cfg, cin, temp);
        for (int i = 0; i < data_len; i++)
        {
            restored_data[i] = temp[i] / data_len;
        }
        delete[] cin;
        cin = NULL;
        delete[] temp;
        temp = NULL;
        kiss_fftr_free (cfg);
    }
    catch (...)
    {
        if (temp)
        {
            delete[] temp;
        }
        if (cin)
        {
            delete[] cin;
        }
        if (cfg)
        {
            kiss_fftr_free (cfg);
        }
        data_logger->error ("Error with doing inverse FFT.");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int get_psd (double *data, int data_len, int sampling_rate, int window_function,
    double *output_ampl, double *output_freq)
{
    if ((data == NULL) || (sampling_rate < 1) || (data_len < 1) || (data_len % 2 == 1) ||
        (output_ampl == NULL) || (output_freq == NULL))
    {
        data_logger->error ("Please check to make sure all arguments aren't empty, sampling rate "
                            "is >=1 and data_len is even.");
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
        if ((total_cols != 0) && (total_cols != (int)splitted.size ()))
        {
            data_logger->error ("some rows have more cols than others, invalid input file");
            fclose (fp);
            return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
        }
        total_cols = (int)splitted.size ();
        for (int i = 0; i < total_cols; i++)
        {
            try
            {
                data[i * total_rows + current_row] = std::stod (splitted[i]);
            }
            catch (const std::invalid_argument &)
            {
                fclose (fp);
                data_logger->error ("found not a number in data file");
                return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
            }
            if (cur_pos == (num_elements - 1))
            {
                *num_cols = current_row + 1;
                *num_rows = total_cols;
                fclose (fp);
                return (int)BrainFlowExitCodes::STATUS_OK;
            }
            cur_pos++;
        }
        current_row++;
    }
    // more likely code below is unreachable
    *num_cols = total_rows;
    *num_rows = total_cols;
    fclose (fp);
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int calc_stddev (double *data, int start_pos, int end_pos, double *output)
{
    if ((data == NULL) || (output == NULL) || (end_pos - start_pos < 2))
    {
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    double mean = 0;
    for (int i = start_pos; i < end_pos; i++)
    {
        mean += data[i];
    }
    mean /= (end_pos - start_pos);
    double stddev = 0;
    for (int i = start_pos; i < end_pos; i++)
    {
        stddev += (data[i] - mean) * (data[i] - mean);
    }
    stddev /= (end_pos - start_pos);
    *output = sqrt (stddev);
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
    if (detrend_operation == (int)DetrendOperations::NO_DETREND)
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
        double mean_x = (data_len - 1) / 2.0;
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

int get_custom_band_powers (double *raw_data, int rows, int cols, double *start_freqs,
    double *stop_freqs, int num_bands, int sampling_rate, int apply_filters,
    double *avg_band_powers, double *stddev_band_powers)
{
    if ((sampling_rate < 1) || (raw_data == NULL) || (rows < 1) || (cols < 1) ||
        (avg_band_powers == NULL) || (stddev_band_powers == NULL) || (start_freqs == NULL) ||
        (stop_freqs == NULL) || (num_bands < 1))
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
    double **bands = new double *[num_bands];
    for (int i = 0; i < num_bands; i++)
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
            exit_codes[i] = detrend (thread_data, cols, (int)DetrendOperations::CONSTANT);
            if (exit_codes[i] == (int)BrainFlowExitCodes::STATUS_OK)
            {
                exit_codes[i] = perform_bandstop (thread_data, cols, sampling_rate, 48.0, 52.0, 4,
                    (int)FilterTypes::BUTTERWORTH_ZERO_PHASE, 0.0);
            }
            if (exit_codes[i] == (int)BrainFlowExitCodes::STATUS_OK)
            {
                exit_codes[i] = perform_bandstop (thread_data, cols, sampling_rate, 58.0, 62.0, 4,
                    (int)FilterTypes::BUTTERWORTH_ZERO_PHASE, 0.0);
            }
            if (exit_codes[i] == (int)BrainFlowExitCodes::STATUS_OK)
            {
                exit_codes[i] = perform_bandpass (thread_data, cols, sampling_rate, 2.0, 45.0, 4,
                    (int)FilterTypes::BUTTERWORTH_ZERO_PHASE, 0.0);
            }
        }

        // use 80% overlap, as long as it works fast overlap param can be big
        exit_codes[i] = get_psd_welch (thread_data, cols, nfft, 4 * nfft / 5, sampling_rate,
            (int)WindowOperations::HANNING, ampls, freqs);
        for (int band_num = 0; band_num < num_bands; band_num++)
        {
            if (exit_codes[i] == (int)BrainFlowExitCodes::STATUS_OK)
            {
                exit_codes[i] = get_band_power (ampls, freqs, nfft / 2 + 1, start_freqs[band_num],
                    stop_freqs[band_num], &bands[band_num][i]);
            }
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
            for (int j = 0; j < num_bands; j++)
            {
                delete[] bands[j];
            }
            delete[] bands;
            return ec;
        }
    }

    // find average and stddev
    double *avg_bands = new double[num_bands];
    double *std_bands = new double[num_bands];
    memset (avg_bands, 0, sizeof (double) * num_bands);
    memset (std_bands, 0, sizeof (double) * num_bands);
    for (int i = 0; i < num_bands; i++)
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
    for (int i = 0; i < num_bands; i++)
    {
        sum += avg_bands[i];
    }
    for (int i = 0; i < num_bands; i++)
    {
        avg_band_powers[i] = avg_bands[i] / sum;
        // use relative stddev to 'normalize'(doesnt ensure range between 0 and 1) it and keep
        // information about variance, division by max doesnt make any sense for stddev, it will
        // lose information about ratio between mean and deviation
        stddev_band_powers[i] = std_bands[i] / avg_bands[i];
    }

    delete[] exit_codes;
    for (int j = 0; j < num_bands; j++)
    {
        delete[] bands[j];
    }
    delete[] bands;
    delete[] avg_bands;
    delete[] std_bands;

    return (int)BrainFlowExitCodes::STATUS_OK;
}

int get_railed_percentage (double *raw_data, int data_len, int gain, double *output)
{
    if ((raw_data == NULL) || (data_len < 1) || (gain < 1) || (output == NULL))
    {
        data_logger->error ("Please review your arguments.");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    double scaler = (4.5 / (pow (2, 23) - 1) / gain * 1000000.);
    double max_val = scaler * pow (2, 23);
    int cur_max = abs (raw_data[0]);
    bool is_straight_line = true;
    for (int i = 1; i < data_len; i++)
    {
        if (abs (raw_data[i]) > cur_max)
        {
            cur_max = abs (raw_data[i]);
        }
        if (((abs (raw_data[i - 1]) - raw_data[i]) > 0.00001) && (abs (raw_data[i]) > 0.00001))
        {
            is_straight_line = false;
        }
    }

    if (is_straight_line)
    {
        *output = 100.0;
    }
    else
    {
        *output = (cur_max / max_val) * 100;
    }

    return (int)BrainFlowExitCodes::STATUS_OK;
}

int get_oxygen_level (double *ppg_ir, double *ppg_red, int data_size, int sampling_rate,
    double callib_coef1, double callib_coef2, double callib_coef3, double *oxygen_level)
{
    if ((ppg_red == NULL) || (ppg_ir == NULL) || (data_size < 40) || (sampling_rate < 1) ||
        (oxygen_level == NULL))
    {
        data_logger->error ("invalid inputs for get_oxygen_level, ir {}, red {}, size {}, sampling "
                            "{}, output {}, min size is 40",
            (ppg_ir != NULL), (ppg_red != NULL), data_size, sampling_rate, (oxygen_level != NULL));
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    int res = (int)BrainFlowExitCodes::STATUS_OK;

    double *red_raw = new double[data_size];
    double *ir_raw = new double[data_size];
    int filter_shift = 25; // to get rif of filtereing artifact, dont use first elements
    int new_size = data_size - filter_shift;
    double *new_red_raw = red_raw + filter_shift;
    double *new_ir_raw = ir_raw + filter_shift;
    memcpy (red_raw, ppg_red, data_size * sizeof (double));
    memcpy (ir_raw, ppg_ir, data_size * sizeof (double));

    // need prefiltered mean of red and ir for dc
    double mean_red = mean (new_red_raw, new_size);
    double mean_ir = mean (new_ir_raw, new_size);

    // filtering(full size)
    res = detrend (red_raw, data_size, (int)DetrendOperations::CONSTANT);
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        res = detrend (ir_raw, data_size, (int)DetrendOperations::CONSTANT);
    }
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        res = perform_bandpass (
            red_raw, data_size, sampling_rate, 0.7, 1.5, 4, (int)FilterTypes::BUTTERWORTH, 0.0);
    }
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        res = perform_bandpass (
            ir_raw, data_size, sampling_rate, 0.7, 1.5, 4, (int)FilterTypes::BUTTERWORTH, 0.0);
    }

    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        // calculate AC & DC components using mean & rms:
        double redac = rms (new_red_raw, new_size);
        double irac = rms (new_ir_raw, new_size);
        double reddc = mean_red;
        double irdc = mean_ir;

        // https://www.maximintegrated.com/en/design/technical-documents/app-notes/6/6845.html
        double r = (redac / reddc) / (irac / irdc);
        data_logger->trace ("r is: {}", r);
        double spo2 = callib_coef1 * r * r + callib_coef2 * r + callib_coef3;
        if (spo2 > 100.0)
        {
            spo2 = 100.0;
        }
        if (spo2 < 0)
        {
            spo2 = 0.0;
        }
        *oxygen_level = spo2;
    }

    delete[] red_raw;
    delete[] ir_raw;

    return (int)BrainFlowExitCodes::STATUS_OK;
}

int restore_data_from_wavelet_detailed_coeffs (double *data, int data_len, int wavelet,
    int decomposition_level, int level_to_restore, double *output)
{
    int extension = (int)WaveletExtensionTypes::SYMMETRIC;
    if ((data == NULL) || (data_len <= 20) || (output == NULL) || (decomposition_level <= 0) ||
        (level_to_restore <= 0) || (level_to_restore > decomposition_level))
    {
        data_logger->error ("Invalid input for restore_data_from_wavelet_detailed_coeffs.");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    int max_wavelet_output_len = data_len + 2 * decomposition_level * (40 + 1);
    double *wavelet_output = new double[max_wavelet_output_len];
    int *decomposition_lengths = new int[decomposition_level + 1];

    int res = perform_wavelet_transform (data, data_len, wavelet, decomposition_level, extension,
        wavelet_output, decomposition_lengths);
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        // zero approx coefs
        for (int j = 0; j < decomposition_lengths[0]; j++)
        {
            wavelet_output[j] = 0.0;
        }
        int cur_sum = decomposition_lengths[0];
        // zero detailed coefs not from level_to_restore
        for (int i = 1; i < decomposition_level + 1; i++)
        {
            int cur_level = decomposition_level + 1 - i;
            if (cur_level != level_to_restore)
            {
                for (int j = cur_sum; j < cur_sum + decomposition_lengths[i]; j++)
                {
                    wavelet_output[j] = 0.0;
                }
            }
            cur_sum += decomposition_lengths[i];
        }
        res = perform_inverse_wavelet_transform (wavelet_output, data_len, wavelet,
            decomposition_level, extension, decomposition_lengths, output);
    }

    delete[] wavelet_output;
    delete[] decomposition_lengths;

    return res;
}

// https://stackoverflow.com/a/22640362
int detect_peaks_z_score (
    double *data, int data_len, int lag, double threshold, double influence, double *output)
{
    if ((data == NULL) || (data_len < lag) || (lag < 2) || (lag > data_len) || (threshold < 0) ||
        (influence < 0) || (output == NULL))
    {
        data_logger->error ("invalid inputs for detect_peaks_z_score");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    memset (output, 0, sizeof (double) * data_len);
    double *filtered_data = new double[data_len];
    double *avg_filter = new double[data_len];
    double *std_filter = new double[data_len];
    memcpy (filtered_data, data, sizeof (double) * data_len);

    avg_filter[lag - 1] = mean (data, lag);
    std_filter[lag - 1] = stddev (data, lag);

    for (int i = lag; i < data_len; i++)
    {
        if (abs (data[i] - avg_filter[i - 1]) > threshold * std_filter[i - 1])
        {
            if (data[i] > avg_filter[i - 1])
            {
                output[i] = 1;
            }
            else
            {
                output[i] = -1;
            }
            filtered_data[i] = influence * data[i] + (1 - influence) * filtered_data[i - 1];
        }
        else
        {
            output[i] = 0;
        }
        avg_filter[i] = mean (filtered_data + i - lag, lag);
        std_filter[i] = stddev (filtered_data + i - lag, lag);
    }

    delete[] filtered_data;
    delete[] avg_filter;
    delete[] std_filter;

    return (int)BrainFlowExitCodes::STATUS_OK;
}

int get_heart_rate (
    double *ppg_ir, double *ppg_red, int data_size, int sampling_rate, int fft_size, double *rate)
{
    if ((ppg_red == NULL) || (ppg_ir == NULL) || (data_size < fft_size) || (sampling_rate < 1) ||
        (rate == NULL) || (fft_size < 1024) || (fft_size % 2 != 0))
    {
        data_logger->error (
            "invalid inputs for get_heart_rate, fft_len should be even and at least 1024");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    int psd_size = fft_size / 2 + 1;
    double *output_ampl_ir = new double[psd_size];
    double *output_ampl_red = new double[psd_size];
    double *output_freq = new double[psd_size];

    int res = get_psd_welch (ppg_ir, data_size, fft_size, fft_size / 2, sampling_rate,
        (int)WindowOperations::HANNING, output_ampl_ir, output_freq);
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        res = get_psd_welch (ppg_red, data_size, fft_size, fft_size / 2, sampling_rate,
            (int)WindowOperations::HANNING, output_ampl_red, output_freq);
    }
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        // calc HR using red/ir psd. HR range 35bpm-230bpm
        // average ampls for red and ir, store in red
        for (int i = 0; i < psd_size; i++)
        {
            output_ampl_red[i] = (output_ampl_red[i] + output_ampl_ir[i]) / 2;
        }
        double min_hr = 35.0 / 60.0;
        double max_hr = 230.0 / 60.0;
        // find max amplitude
        double max_ampl = 0.0;
        int max_ampl_index = 0;
        for (int i = 0; i < psd_size; i++)
        {
            if (output_freq[i] > min_hr && output_freq[i] < max_hr && output_ampl_red[i] > max_ampl)
            {
                max_ampl = output_ampl_red[i];
                max_ampl_index = i;
            }
            else if (output_freq[i] > max_hr)
            {
                break;
            }
        }
        double heart_rate = output_freq[max_ampl_index] * 60;
        *rate = heart_rate;
    }

    delete[] output_ampl_ir;
    delete[] output_ampl_red;
    delete[] output_freq;

    return res;
}

int perform_ica (double *data, int rows, int cols, int num_components, double *w_mat, double *k_mat,
    double *a_mat, double *s_mat)
{
    if ((data == NULL) || (rows < 2) || (cols < 2) || (num_components < 2) || (w_mat == NULL) ||
        (k_mat == NULL) || (a_mat == NULL) || (s_mat == NULL))
    {
        data_logger->error ("invalid inputs for perform_ica.");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    Eigen::MatrixXd input_matrix =
        Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>> (
            data, rows, cols);

    FastICA ica (num_components);
    int res = ica.compute (input_matrix);
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        res = ica.get_matrixes (w_mat, k_mat, a_mat, s_mat);
    }
    return res;
}

int get_version_data_handler (char *version, int *num_chars, int max_chars)
{
    strncpy (version, BRAINFLOW_VERSION_STRING, max_chars);
    *num_chars = std::min<int> (max_chars, (int)strlen (BRAINFLOW_VERSION_STRING));
    return (int)BrainFlowExitCodes::STATUS_OK;
}
