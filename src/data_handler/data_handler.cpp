#include <algorithm>
#include <cmath>
#include <limits>
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
#include "filter_helpers.h"

#include "window_functions.h"

#include "DspFilters/Dsp.h"

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


// inside wavelib inverse transform uses internal state from direct transform, dirty hack to restore
// it here

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


int get_band_power (double *ampl, double *freq, int data_len, double freq_start, double freq_end,
    double *band_power)
{
    if ((ampl == NULL) || (freq == NULL) || (band_power == NULL) || (data_len < 2) ||
        !std::isfinite (freq_start) || !std::isfinite (freq_end) || (freq_start < 0.0) ||
        (freq_start >= freq_end))
    {
        data_logger->error ("Band bounds must be finite and satisfy 0 <= start < end.");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    for (int i = 0; i < data_len; i++)
    {
        if (!std::isfinite (freq[i]) || !std::isfinite (ampl[i]) || (ampl[i] < 0.0) ||
            (freq[i] < 0.0) || ((i > 0) && (freq[i] <= freq[i - 1])))
        {
            data_logger->error ("PSD must be finite and nonnegative with increasing frequencies.");
            return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
        }
    }
    if ((freq_start < freq[0]) || (freq_end > freq[data_len - 1]))
    {
        data_logger->error ("Band bounds must be within the PSD frequency range.");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    double res = 0.0;
    for (int i = 0; (i < data_len - 1) && (freq[i] < freq_end); i++)
    {
        const double left = std::max (freq_start, freq[i]);
        const double right = std::min (freq_end, freq[i + 1]);
        if (right > left)
        {
            const double width = freq[i + 1] - freq[i];
            const double left_fraction = (left - freq[i]) / width;
            const double right_fraction = (right - freq[i]) / width;
            const double left_power = (1.0 - left_fraction) * ampl[i] + left_fraction * ampl[i + 1];
            const double right_power =
                (1.0 - right_fraction) * ampl[i] + right_fraction * ampl[i + 1];
            res += (right - left) * (0.5 * left_power + 0.5 * right_power);
        }
    }
    if (!std::isfinite (res))
    {
        data_logger->error ("Band power overflowed.");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    *band_power = res;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

namespace
{
    bool prepare_band_power (int fs, int apply, int &nfft, double low, double high, int mains,
        std::vector<band_power_helpers::Section> &sections, int &guard)
    {
        if (fs < 1 || fs > std::numeric_limits<int>::max () / 4 || apply < 0 || apply > 1 ||
            !std::isfinite (low) || !std::isfinite (high) || low < 0 || high < 0 ||
            low >= fs / 2.0 || high >= fs / 2.0 || (low > 0 && high > 0 && low >= high) ||
            mains < -1 || mains > 3)
            return false;
        if (nfft == 0)
        {
            if (get_nearest_power_of_two (fs, &nfft))
                return false;
            nfft = std::max (8, 2 * nfft);
        }
        if (nfft < 2 || (nfft & (nfft - 1)))
            return false;
        guard = 0;
        if (!apply)
            return true;
        double radius = 0.0;
        for (int i = 0; i < 2; i++)
        {
            const double center = i == 0 ? 50.0 : 60.0;
            // Automatic mains removal excludes the poorly conditioned region next to Nyquist.
            const bool enabled = mains == -1 ? center + 2 < 0.9 * fs / 2.0 : (mains & (1 << i));
            if (enabled &&
                !filter_helpers::design (3, fs, center - 2, center + 2, 4, 0, 0, sections, radius))
                return false;
        }
        if (low > 0 && high > 0)
        {
            if (!filter_helpers::design (2, fs, low, high, 4, 0, 0, sections, radius))
                return false;
        }
        else if (low > 0 || high > 0)
        {
            if (!filter_helpers::design (
                    low > 0 ? 1 : 0, fs, low > 0 ? low : high, 0, 4, 0, 0, sections, radius))
                return false;
        }
        return filter_helpers::settling_samples (sections, radius, guard);
    }
}

int get_band_power_settings (int fs, int apply, int nfft, double low, double high, int mains,
    int *effective_nfft, int *edge_samples)
try
{
    if (!effective_nfft || !edge_samples)
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    std::vector<band_power_helpers::Section> sections;
    int guard;
    if (!prepare_band_power (fs, apply, nfft, low, high, mains, sections, guard) ||
        (int64_t)nfft + 2LL * guard > std::numeric_limits<int>::max ())
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    *effective_nfft = nfft;
    *edge_samples = guard;
    return (int)BrainFlowExitCodes::STATUS_OK;
}
catch (...)
{
    return (int)BrainFlowExitCodes::GENERAL_ERROR;
}

int get_custom_band_powers (double *raw_data, int rows, int cols, double *start_freqs,
    double *stop_freqs, int num_bands, int sampling_rate, int apply_filters,
    double *avg_band_powers, double *stddev_band_powers)
{
    return get_custom_band_powers_with_options (raw_data, rows, cols, start_freqs, stop_freqs,
        num_bands, sampling_rate, apply_filters, 0, 0, 0, -1, apply_filters ? 1 : 0,
        avg_band_powers, stddev_band_powers);
}

int get_custom_band_powers_with_options (double *raw_data, int rows, int cols, double *start_freqs,
    double *stop_freqs, int num_bands, int sampling_rate, int apply_filters, int nfft,
    double low_cutoff, double high_cutoff, int mains, int detrend_operation,
    double *avg_band_powers, double *stddev_band_powers)
try
{
    if ((sampling_rate < 1) || (sampling_rate > std::numeric_limits<int>::max () / 4) ||
        (raw_data == NULL) || (rows < 1) || (cols < 1) || (avg_band_powers == NULL) ||
        (stddev_band_powers == NULL) || (start_freqs == NULL) || (stop_freqs == NULL) ||
        (num_bands < 1) || detrend_operation < 0 || detrend_operation > 2 ||
        (int64_t)rows * cols > std::numeric_limits<int>::max () ||
        (int64_t)rows * num_bands > std::numeric_limits<int>::max ())
    {
        data_logger->error ("Please review your arguments.");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    const double nyquist = sampling_rate / 2.0;
    for (int i = 0; i < num_bands; i++)
    {
        if (!std::isfinite (start_freqs[i]) || !std::isfinite (stop_freqs[i]) ||
            (start_freqs[i] < 0.0) || (start_freqs[i] >= stop_freqs[i]) ||
            (stop_freqs[i] > nyquist))
        {
            data_logger->error ("Bands must satisfy 0 <= start < end <= Nyquist.");
            return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
        }
    }

    int guard = 0;
    std::vector<band_power_helpers::Section> sections;
    if (!prepare_band_power (
            sampling_rate, apply_filters, nfft, low_cutoff, high_cutoff, mains, sections, guard))
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    const int64_t required = (int64_t)nfft + 2 * (int64_t)guard;
    if (cols < required)
    {
        data_logger->error ("Band powers need at least {} samples: {} for Welch and {} at each "
                            "edge for filter settling; received {}.",
            required, nfft, guard, cols);
        return (int)BrainFlowExitCodes::INVALID_BUFFER_SIZE_ERROR;
    }
    const int retained = cols - 2 * guard;
    std::vector<int> exit_codes (rows, (int)BrainFlowExitCodes::STATUS_OK);
    std::vector<std::vector<double>> bands (num_bands, std::vector<double> (rows, 0.0));

#pragma omp parallel for
    for (int i = 0; i < rows; i++)
    {
        try
        {
            const double *channel = raw_data + (size_t)i * cols;
            std::vector<double> thread_data (channel, channel + cols);
            for (double sample : thread_data)
            {
                if (!std::isfinite (sample))
                {
                    exit_codes[i] = (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
                    break;
                }
            }
            if (exit_codes[i] != (int)BrainFlowExitCodes::STATUS_OK)
            {
                continue;
            }
            exit_codes[i] = detrend (thread_data.data (), cols, detrend_operation);
            if (exit_codes[i] != (int)BrainFlowExitCodes::STATUS_OK)
                continue;
            if (!sections.empty () &&
                !band_power_helpers::filter_with_padding (thread_data, sections, guard))
            {
                exit_codes[i] = (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
                continue;
            }

            std::vector<double> ampls (nfft / 2 + 1);
            std::vector<double> freqs (nfft / 2 + 1);
            // Filter the complete input first, then exclude the settling margins from Welch.
            exit_codes[i] =
                get_psd_welch (thread_data.data () + guard, retained, nfft, (int)(4LL * nfft / 5),
                    sampling_rate, (int)WindowOperations::HANNING, ampls.data (), freqs.data ());
            for (int band_num = 0; band_num < num_bands; band_num++)
            {
                if (exit_codes[i] != (int)BrainFlowExitCodes::STATUS_OK)
                {
                    break;
                }
                exit_codes[i] = get_band_power (ampls.data (), freqs.data (), nfft / 2 + 1,
                    start_freqs[band_num], stop_freqs[band_num], &bands[band_num][i]);
            }
        }
        catch (...)
        {
            exit_codes[i] = (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
    }

    for (int code : exit_codes)
    {
        if (code != (int)BrainFlowExitCodes::STATUS_OK)
        {
            return code;
        }
    }

    // A common scale preserves channel weighting while avoiding overflow when accumulating
    // powers or squaring deviations. This is still mean absolute power followed by normalization.
    double scale = 0.0;
    for (const auto &band : bands)
    {
        for (double power : band)
        {
            scale = std::max (scale, power);
        }
    }
    if (scale == 0.0)
    {
        std::fill (avg_band_powers, avg_band_powers + num_bands, 0.0);
        std::fill (stddev_band_powers, stddev_band_powers + num_bands, 0.0);
        return (int)BrainFlowExitCodes::STATUS_OK;
    }

    std::vector<double> means (num_bands, 0.0);
    std::vector<double> deviations (num_bands, 0.0);
    double total = 0.0;
    for (int i = 0; i < num_bands; i++)
    {
        for (double power : bands[i])
        {
            means[i] += (power / scale) / rows;
        }
        for (double power : bands[i])
        {
            const double difference = power / scale - means[i];
            deviations[i] += difference * difference / rows;
        }
        total += means[i];
    }
    for (int i = 0; i < num_bands; i++)
    {
        avg_band_powers[i] = means[i] / total;
        // The second output is the population coefficient of variation of absolute channel
        // powers, not the standard deviation of the normalized averages. Define silent bands as 0.
        stddev_band_powers[i] = (means[i] > 0.0) ? std::sqrt (deviations[i]) / means[i] : 0.0;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

catch (...)
{
    return (int)BrainFlowExitCodes::GENERAL_ERROR;
}


int get_version_data_handler (char *version, int *num_chars, int max_chars)
{
    strncpy (version, BRAINFLOW_VERSION_STRING, max_chars);
    *num_chars = std::min<int> (max_chars, (int)strlen (BRAINFLOW_VERSION_STRING));
    return (int)BrainFlowExitCodes::STATUS_OK;
}
