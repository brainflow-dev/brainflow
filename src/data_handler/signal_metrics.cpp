#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>
#include <vector>

#include "brainflow_constants.h"
#include "common_data_handler_helpers.h"
#include "data_handler.h"
#include "filter_helpers.h"
#include "spdlog/spdlog.h"

extern std::shared_ptr<spdlog::logger> data_logger;

namespace
{
    const double minimum_pulse_hz = 35.0 / 60.0;
    const double maximum_pulse_hz = 230.0 / 60.0;

    int insufficient_pulse ()
    {
        data_logger->error ("PPG has insufficient pulse energy or no distinct shared pulse peak");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    bool center_ppg (const double *data, int count, std::vector<double> &centered)
    {
        double scale = signal_scale (data, count);
        if (!(scale > 0.0))
        {
            return false;
        }
        centered.resize (count);
        for (int i = 0; i < count; i++)
        {
            centered[i] = data[i] / scale;
        }
        if (detrend (centered.data (), count, (int)DetrendOperations::LINEAR) !=
            (int)BrainFlowExitCodes::STATUS_OK)
        {
            return false;
        }
        return stddev (centered.data (), count) > 64 * std::numeric_limits<double>::epsilon ();
    }

    int pulse_spectrum (double *data, int count, int sampling_rate, int fft_size,
        std::vector<double> &spectrum, std::vector<double> &frequencies, bool periodogram = false)
    {
        spectrum.resize (fft_size / 2 + 1);
        frequencies.resize (spectrum.size ());
        int result = periodogram ?
            get_psd (data, fft_size, sampling_rate, (int)WindowOperations::HANNING,
                spectrum.data (), frequencies.data ()) :
            get_psd_welch (data, count, fft_size, fft_size / 2, sampling_rate,
                (int)WindowOperations::HANNING, spectrum.data (), frequencies.data ());
        if (result != (int)BrainFlowExitCodes::STATUS_OK)
        {
            return result;
        }
        double total = 0.0;
        for (size_t i = 0; i < spectrum.size (); i++)
        {
            if (frequencies[i] >= minimum_pulse_hz && frequencies[i] <= maximum_pulse_hz)
            {
                total += spectrum[i];
            }
        }
        if (!std::isfinite (total) || !(total > std::numeric_limits<double>::min ()))
        {
            return insufficient_pulse ();
        }
        for (double &value : spectrum)
        {
            value /= total;
        }
        return (int)BrainFlowExitCodes::STATUS_OK;
    }

    bool pulse_peak (
        const std::vector<double> &spectrum, const std::vector<double> &frequencies, int &peak)
    {
        std::vector<double> background;
        double total = 0.0;
        peak = -1;
        for (int i = 1; i + 1 < (int)spectrum.size (); i++)
        {
            if (frequencies[i] >= minimum_pulse_hz && frequencies[i] <= maximum_pulse_hz)
            {
                background.push_back (spectrum[i]);
                total += spectrum[i];
                if (peak < 0 || spectrum[i] > spectrum[peak])
                {
                    peak = i;
                }
            }
        }
        if (background.size () < 5 || peak < 0 || !(total > 0.0))
        {
            return false;
        }
        std::nth_element (
            background.begin (), background.begin () + background.size () / 2, background.end ());
        double median = background[background.size () / 2];
        double local = spectrum[peak - 1] + spectrum[peak] + spectrum[peak + 1];
        // A Hann-windowed pulse should concentrate power around a peak. This numerical
        // quality gate rejects flat/noisy spectra; it is not a clinical validity claim.
        return spectrum[peak] > 4.0 * median && local >= 0.45 * total &&
            spectrum[peak] >= spectrum[peak - 1] && spectrum[peak] >= spectrum[peak + 1];
    }
}

int get_railed_percentage (double *raw_data, int data_len, int gain, double *output)
{
    if (!output || gain < 1 || !finite_signal (raw_data, data_len))
    {
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    bool flat = true;
    for (int i = 1; i < data_len; i++)
    {
        if (std::abs (raw_data[i - 1] - raw_data[i]) > 0.00001 && std::abs (raw_data[i]) > 0.00001)
        {
            flat = false;
        }
    }
    // Preserve the legacy ADS1299 4.5 V, 24-bit, microvolt peak metric and its flatline
    // convention. get_clipping_percentage measures an actual clipped sample fraction.
    double full_scale = (4.5 / (8388608.0 - 1.0) / gain * 1000000.0) * 8388608.0;
    double fraction = std::min (1.0, signal_scale (raw_data, data_len) / full_scale);
    *output = flat ? 100.0 : 100.0 * fraction;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int get_clipping_percentage (
    const double *data, int data_len, double lower_bound, double upper_bound, double *output)
{
    if (!output || !std::isfinite (lower_bound) || !std::isfinite (upper_bound) ||
        lower_bound >= upper_bound || !finite_signal (data, data_len))
    {
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    int count = 0;
    for (int i = 0; i < data_len; i++)
    {
        if (data[i] <= lower_bound || data[i] >= upper_bound)
        {
            count++;
        }
    }
    *output = 100.0 * ((double)count / data_len);
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int get_flatline_percentage (const double *data, int data_len, double tolerance, double *output)
{
    if (!output || data_len < 2 || !std::isfinite (tolerance) || tolerance < 0.0 ||
        !finite_signal (data, data_len))
    {
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    int count = 0;
    for (int i = 1; i < data_len; i++)
    {
        if (std::abs (data[i] - data[i - 1]) <= tolerance)
        {
            count++;
        }
    }
    *output = 100.0 * ((double)count / (data_len - 1));
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int get_oxygen_level (double *ppg_ir, double *ppg_red, int data_size, int sampling_rate,
    double coefficient1, double coefficient2, double coefficient3, double *oxygen_level)
{
    if (!oxygen_level || sampling_rate <= 8 || !std::isfinite (coefficient1) ||
        !std::isfinite (coefficient2) || !std::isfinite (coefficient3) ||
        !finite_signal (ppg_ir, data_size) || !finite_signal (ppg_red, data_size))
    {
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    try
    {
        // The previous 0.7-1.5 Hz passband excluded otherwise supported pulse rates.
        std::vector<filter_helpers::Section> sections;
        double radius = 0.0;
        int guard = 0;
        if (!filter_helpers::design (2, sampling_rate, 0.5, 4.0, 2,
                (int)FilterTypes::BUTTERWORTH_ZERO_PHASE, 0.0, sections, radius) ||
            !filter_helpers::settling_samples (sections, radius, guard))
        {
            return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
        }
        if ((long long)data_size < 2LL * guard + 4LL * sampling_rate)
        {
            data_logger->error (
                "Oxygen estimation needs at least {} samples including {} at each boundary",
                2LL * guard + 4LL * sampling_rate, guard);
            return (int)BrainFlowExitCodes::INVALID_BUFFER_SIZE_ERROR;
        }
        int retained = data_size - 2 * guard;
        double ir_scale = signal_scale (ppg_ir, data_size);
        double red_scale = signal_scale (ppg_red, data_size);
        std::vector<double> ir, red;
        if (!center_ppg (ppg_ir, data_size, ir) || !center_ppg (ppg_red, data_size, red))
        {
            return insufficient_pulse ();
        }
        double dc_ir = mean (ppg_ir + guard, retained) / ir_scale;
        double dc_red = mean (ppg_red + guard, retained) / red_scale;
        const double minimum_fraction = 64 * std::numeric_limits<double>::epsilon ();
        if (!(dc_ir > minimum_fraction) || !(dc_red > minimum_fraction))
        {
            return insufficient_pulse ();
        }
        if (!band_power_helpers::filter_with_padding (ir, sections, guard) ||
            !band_power_helpers::filter_with_padding (red, sections, guard))
        {
            return (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
        int fft_size = retained - retained % 2;
        std::vector<double> ir_spectrum, red_spectrum, frequencies;
        int result = pulse_spectrum (
            ir.data () + guard, retained, sampling_rate, fft_size, ir_spectrum, frequencies, true);
        if (result != (int)BrainFlowExitCodes::STATUS_OK)
        {
            return result;
        }
        result = pulse_spectrum (red.data () + guard, retained, sampling_rate, fft_size,
            red_spectrum, frequencies, true);
        if (result != (int)BrainFlowExitCodes::STATUS_OK)
        {
            return result;
        }
        int ir_peak, red_peak;
        if (!pulse_peak (ir_spectrum, frequencies, ir_peak) ||
            !pulse_peak (red_spectrum, frequencies, red_peak) || std::abs (ir_peak - red_peak) > 1)
        {
            return insufficient_pulse ();
        }
        double ac_ir = rms (ir.data () + guard, retained);
        double ac_red = rms (red.data () + guard, retained);
        if (!(ac_ir > minimum_fraction) || !(ac_red > minimum_fraction))
        {
            return insufficient_pulse ();
        }
        double ratio = (ac_red / dc_red) / (ac_ir / dc_ir);
        double saturation = (coefficient1 * ratio + coefficient2) * ratio + coefficient3;
        if (!std::isfinite (ratio) || !std::isfinite (saturation))
        {
            return (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
        *oxygen_level = std::max (0.0, std::min (100.0, saturation));
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    catch (const std::exception &error)
    {
        data_logger->error ("Oxygen estimation failed: {}", error.what ());
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
}

int get_heart_rate (
    double *ppg_ir, double *ppg_red, int data_size, int sampling_rate, int fft_size, double *rate)
{
    if (!rate || sampling_rate < 8 || fft_size < 1024 || fft_size % 2 != 0 ||
        data_size < fft_size || (double)sampling_rate / fft_size > 0.25 ||
        !finite_signal (ppg_ir, data_size) || !finite_signal (ppg_red, data_size))
    {
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    try
    {
        std::vector<double> ir, red;
        if (!center_ppg (ppg_ir, data_size, ir) || !center_ppg (ppg_red, data_size, red))
        {
            return insufficient_pulse ();
        }
        std::vector<double> ir_spectrum, red_spectrum, frequencies;
        int result = pulse_spectrum (
            ir.data (), data_size, sampling_rate, fft_size, ir_spectrum, frequencies);
        if (result != (int)BrainFlowExitCodes::STATUS_OK)
        {
            return result;
        }
        result = pulse_spectrum (
            red.data (), data_size, sampling_rate, fft_size, red_spectrum, frequencies);
        if (result != (int)BrainFlowExitCodes::STATUS_OK)
        {
            return result;
        }
        int ir_peak, red_peak;
        if (!pulse_peak (ir_spectrum, frequencies, ir_peak) ||
            !pulse_peak (red_spectrum, frequencies, red_peak) || std::abs (ir_peak - red_peak) > 1)
        {
            return insufficient_pulse ();
        }
        for (size_t i = 0; i < ir_spectrum.size (); i++)
        {
            ir_spectrum[i] = (ir_spectrum[i] + red_spectrum[i]) / 2.0;
        }
        int peak;
        if (!pulse_peak (ir_spectrum, frequencies, peak))
        {
            return insufficient_pulse ();
        }
        // Log-parabolic interpolation reduces FFT-bin quantization of a validated peak.
        double offset = 0.0;
        if (ir_spectrum[peak - 1] > 0.0 && ir_spectrum[peak + 1] > 0.0)
        {
            double left = std::log (ir_spectrum[peak - 1]);
            double middle = std::log (ir_spectrum[peak]);
            double right = std::log (ir_spectrum[peak + 1]);
            double curvature = left - 2.0 * middle + right;
            if (curvature < 0.0)
            {
                offset = std::max (-0.5, std::min (0.5, 0.5 * (left - right) / curvature));
            }
        }
        double frequency = (peak + offset) * ((double)sampling_rate / fft_size);
        if (frequency < minimum_pulse_hz || frequency > maximum_pulse_hz)
        {
            return insufficient_pulse ();
        }
        *rate = frequency * 60.0;
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    catch (const std::exception &error)
    {
        data_logger->error ("Heart-rate estimation failed: {}", error.what ());
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
}

int detect_peaks_z_score (
    double *data, int data_len, int lag, double threshold, double influence, double *output)
{
    if (!output || lag < 2 || lag > data_len || !std::isfinite (threshold) || threshold < 0.0 ||
        !std::isfinite (influence) || influence < 0.0 || influence > 1.0 ||
        !finite_signal (data, data_len))
    {
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    try
    {
        double scale = signal_scale (data, data_len);
        if (scale == 0.0)
        {
            std::fill (output, output + data_len, 0.0);
            return (int)BrainFlowExitCodes::STATUS_OK;
        }
        std::vector<double> filtered (data_len), peaks (data_len, 0.0);
        for (int i = 0; i < data_len; i++)
        {
            filtered[i] = data[i] / scale;
        }
        double baseline = mean (filtered.data (), lag);
        double deviation = stddev (filtered.data (), lag);
        double squared_deviation = deviation * deviation * lag;
        for (int i = lag; i < data_len; i++)
        {
            double current = data[i] / scale;
            double limit = threshold * std::sqrt (std::max (0.0, squared_deviation) / lag);
            if (std::abs (current - baseline) >
                limit + 32 * std::numeric_limits<double>::epsilon ())
            {
                peaks[i] = current > baseline ? 1.0 : -1.0;
                filtered[i] = influence * current + (1.0 - influence) * filtered[i - 1];
            }
            // The next baseline includes this filtered sample and excludes i-lag.
            double previous = filtered[i - lag];
            double next_baseline = baseline + (filtered[i] - previous) / lag;
            squared_deviation +=
                (filtered[i] - previous) * (filtered[i] - next_baseline + previous - baseline);
            baseline = next_baseline;
            // Periodic recomputation bounds drift while retaining O(data_len) work.
            if ((i - lag + 1) % lag == 0)
            {
                baseline = mean (filtered.data () + i - lag + 1, lag);
                deviation = stddev (filtered.data () + i - lag + 1, lag);
                squared_deviation = deviation * deviation * lag;
            }
        }
        std::copy (peaks.begin (), peaks.end (), output);
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    catch (const std::exception &error)
    {
        data_logger->error ("Peak detection failed: {}", error.what ());
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
}

int get_activity_index (const double *accel_x, const double *accel_y, const double *accel_z,
    int data_len, int sampling_rate, int period, double noise_var_x, double noise_var_y,
    double noise_var_z, double *output)
{
    if (!output || sampling_rate <= 0 || period < sampling_rate || data_len < period ||
        period % sampling_rate != 0 || !std::isfinite (noise_var_x) || noise_var_x < 0.0 ||
        !std::isfinite (noise_var_y) || noise_var_y < 0.0 || !std::isfinite (noise_var_z) ||
        noise_var_z < 0.0 || !finite_signal (accel_x, data_len) ||
        !finite_signal (accel_y, data_len) || !finite_signal (accel_z, data_len))
    {
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    try
    {
        const double noise[] = {
            std::sqrt (noise_var_x), std::sqrt (noise_var_y), std::sqrt (noise_var_z)};
        const double *axes[] = {accel_x, accel_y, accel_z};
        std::vector<double> result (data_len / period, 0.0);
        for (int epoch = 0; epoch < (int)result.size (); epoch++)
        {
            for (int second = 0; second < period / sampling_rate; second++)
            {
                int start = epoch * period + second * sampling_rate;
                double deviations[3];
                double scale = std::max (noise[0], std::max (noise[1], noise[2]));
                for (int axis = 0; axis < 3; axis++)
                {
                    deviations[axis] = stddev (axes[axis] + start, sampling_rate);
                    scale = std::max (scale, deviations[axis]);
                }
                if (scale > 0.0)
                {
                    double variance = 0.0;
                    for (int axis = 0; axis < 3; axis++)
                    {
                        double deviation = deviations[axis] / scale;
                        double floor = noise[axis] / scale;
                        variance += (deviation * deviation - floor * floor) / 3.0;
                    }
                    result[epoch] += scale * std::sqrt (std::max (0.0, variance));
                }
                if (!std::isfinite (result[epoch]))
                {
                    return (int)BrainFlowExitCodes::GENERAL_ERROR;
                }
            }
        }
        std::copy (result.begin (), result.end (), output);
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    catch (const std::exception &error)
    {
        data_logger->error ("Activity index failed: {}", error.what ());
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
}
