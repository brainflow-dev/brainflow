#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <vector>

#include "brainflow_constants.h"
#include "data_handler.h"
#include "spdlog/spdlog.h"

extern std::shared_ptr<spdlog::logger> data_logger;

namespace
{
    bool valid_policy (const BrainFlowSignalQualityParams *p)
    {
        return p && p->struct_size == sizeof (BrainFlowSignalQualityParams) && p->version == 1 &&
            (p->enable_clipping == 0 || p->enable_clipping == 1) &&
            std::isfinite (p->lower_bound) && std::isfinite (p->upper_bound) &&
            (!p->enable_clipping || p->lower_bound < p->upper_bound) &&
            std::isfinite (p->max_clipped_percentage) && p->max_clipped_percentage >= 0 &&
            p->max_clipped_percentage <= 100 && std::isfinite (p->flatline_tolerance) &&
            p->flatline_tolerance >= 0 && std::isfinite (p->max_flatline_seconds) &&
            p->max_flatline_seconds >= 0 && p->min_channels >= 1;
    }
}

int get_signal_quality (const double *data, int rows, int cols, int sampling_rate,
    const BrainFlowSignalQualityParams *params, BrainFlowChannelQuality *output)
try
{
    if (!data || !output || rows < 1 || cols < 2 || sampling_rate < 1 ||
        (int64_t)rows * cols > (std::numeric_limits<int>::max) () || !valid_policy (params))
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;

    for (int row = 0; row < rows; ++row)
    {
        const double *channel = data + (size_t)row * cols;
        int nonfinite = 0, clipped = 0, flat = 0, run = 0, longest = 0;
        for (int i = 0; i < cols; ++i)
        {
            if (!std::isfinite (channel[i]))
            {
                ++nonfinite;
                run = 0;
                continue;
            }
            if (params->enable_clipping &&
                (channel[i] <= params->lower_bound || channel[i] >= params->upper_bound))
                ++clipped;
            if (i > 0 && std::isfinite (channel[i - 1]) &&
                std::abs (channel[i] - channel[i - 1]) <= params->flatline_tolerance)
            {
                ++flat;
                longest = std::max (longest, ++run);
            }
            else
                run = 0;
        }
        BrainFlowChannelQuality q = {};
        q.nonfinite_percentage = 100.0 * nonfinite / cols;
        q.clipped_percentage = 100.0 * clipped / cols;
        q.flatline_percentage = 100.0 * flat / (cols - 1);
        q.longest_flatline_seconds = (double)longest / sampling_rate;
        if (nonfinite)
            q.flags |= BRAINFLOW_QUALITY_NONFINITE;
        if (params->enable_clipping && q.clipped_percentage > params->max_clipped_percentage)
            q.flags |= BRAINFLOW_QUALITY_CLIPPED;
        if (params->max_flatline_seconds > 0 &&
            q.longest_flatline_seconds >= params->max_flatline_seconds)
            q.flags |= BRAINFLOW_QUALITY_FLATLINE;
        output[row] = q;
        // Opt-in diagnostics only. Stateful rejection/recovery logging belongs to callers.
        if (q.flags)
            data_logger->debug (
                "signal_quality channel={} flags={} nonfinite_pct={} clipped_pct={} "
                "flatline_pct={} longest_flatline_seconds={}",
                row, q.flags, q.nonfinite_percentage, q.clipped_percentage, q.flatline_percentage,
                q.longest_flatline_seconds);
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}
catch (...)
{
    return (int)BrainFlowExitCodes::GENERAL_ERROR;
}

int get_custom_band_powers_with_quality (const double *data, int rows, int cols,
    const double *start_freqs, const double *stop_freqs, int num_bands, int sampling_rate,
    int apply_filters, const BrainFlowSignalQualityParams *params, double *avg_band_powers,
    double *stddev_band_powers, BrainFlowChannelQuality *quality, int *valid)
try
{
    if (valid)
        *valid = 0;
    if (!data || !start_freqs || !stop_freqs || !avg_band_powers || !stddev_band_powers ||
        !quality || !valid || rows < 1 || cols < 2 || num_bands < 1 || sampling_rate < 1 ||
        sampling_rate > (std::numeric_limits<int>::max) () / 4 ||
        (int64_t)rows * cols > (std::numeric_limits<int>::max) () ||
        (int64_t)rows * num_bands > (std::numeric_limits<int>::max) () ||
        (apply_filters != 0 && apply_filters != 1) || !valid_policy (params) ||
        params->min_channels > rows)
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    for (int i = 0; i < num_bands; ++i)
        if (!std::isfinite (start_freqs[i]) || !std::isfinite (stop_freqs[i]) ||
            start_freqs[i] < 0 || start_freqs[i] >= stop_freqs[i] ||
            stop_freqs[i] > sampling_rate / 2.0)
            return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;

    // Validate DSP settings/length even when all channels will be rejected.
    int nfft = 0, guard = 0;
    int ec = get_band_power_settings (sampling_rate, apply_filters, 0, 0, 0, -1, &nfft, &guard);
    if (ec != (int)BrainFlowExitCodes::STATUS_OK)
        return ec;
    if ((int64_t)cols < (int64_t)nfft + 2LL * guard)
        return (int)BrainFlowExitCodes::INVALID_BUFFER_SIZE_ERROR;
    ec = get_signal_quality (data, rows, cols, sampling_rate, params, quality);
    if (ec != (int)BrainFlowExitCodes::STATUS_OK)
        return ec;

    std::fill (
        avg_band_powers, avg_band_powers + num_bands, std::numeric_limits<double>::quiet_NaN ());
    std::fill (stddev_band_powers, stddev_band_powers + num_bands,
        std::numeric_limits<double>::quiet_NaN ());
    int accepted = 0;
    for (int row = 0; row < rows; ++row)
        if (quality[row].flags == BRAINFLOW_QUALITY_GOOD)
            ++accepted;
    if (accepted < params->min_channels)
        return (int)BrainFlowExitCodes::STATUS_OK;

    // Keep all sample positions of accepted channels. Never splice across rejected samples.
    std::vector<double> selected;
    selected.reserve ((size_t)accepted * cols);
    for (int row = 0; row < rows; ++row)
        if (quality[row].flags == BRAINFLOW_QUALITY_GOOD)
            selected.insert (
                selected.end (), data + (size_t)row * cols, data + (size_t)(row + 1) * cols);
    std::vector<double> starts (start_freqs, start_freqs + num_bands);
    std::vector<double> stops (stop_freqs, stop_freqs + num_bands);
    ec = get_custom_band_powers (selected.data (), accepted, cols, starts.data (), stops.data (),
        num_bands, sampling_rate, apply_filters, avg_band_powers, stddev_band_powers);
    if (ec == (int)BrainFlowExitCodes::STATUS_OK)
    {
        *valid = std::any_of (
            avg_band_powers, avg_band_powers + num_bands, [] (double power) { return power > 0; });
        if (!*valid)
        {
            std::fill (avg_band_powers, avg_band_powers + num_bands,
                std::numeric_limits<double>::quiet_NaN ());
            std::fill (stddev_band_powers, stddev_band_powers + num_bands,
                std::numeric_limits<double>::quiet_NaN ());
        }
    }
    return ec;
}
catch (...)
{
    return (int)BrainFlowExitCodes::GENERAL_ERROR;
}
