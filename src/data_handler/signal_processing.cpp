#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <limits>
#include <locale>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <sstream>
#include <vector>

#include "brainflow_constants.h"
#include "common_data_handler_helpers.h"
#include "data_handler.h"
#include "filter_helpers.h"
#include "kiss_fftr.h"

namespace
{
    const int ok = (int)BrainFlowExitCodes::STATUS_OK;
    const int invalid = (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    const int failure = (int)BrainFlowExitCodes::GENERAL_ERROR;
    const int short_buffer = (int)BrainFlowExitCodes::INVALID_BUFFER_SIZE_ERROR;

    struct StreamingFilterState
    {
        std::vector<filter_helpers::Section> sections;
        std::vector<double> z1, z2;
        std::mutex mutex;
    };
    std::mutex streaming_registry_mutex;
    std::map<int, std::shared_ptr<StreamingFilterState>> streaming_filters;
    int64_t next_streaming_handle = 1;

    std::shared_ptr<StreamingFilterState> find_streaming_filter (int handle)
    {
        std::lock_guard<std::mutex> lock (streaming_registry_mutex);
        auto found = streaming_filters.find (handle);
        return found == streaming_filters.end () ? nullptr : found->second;
    }

    int filter_signal (double *data, int n, int fs, double low, double high, int order, int type,
        double ripple, int kind)
    {
        if (!finite_signal (data, n))
            return invalid;
        std::vector<filter_helpers::Section> sections;
        double radius = 0.0;
        if (!filter_helpers::design (kind, fs, low, high, order, type, ripple, sections, radius))
            return invalid;
        std::vector<double> result (data, data + n);
        const double scale = signal_scale (data, n);
        if (scale > 0.0)
            for (double &sample : result)
                sample /= scale;
        if (type >= 3)
        {
            int guard;
            if (!filter_helpers::settling_samples (sections, radius, guard) ||
                !band_power_helpers::filter_with_padding (result, sections, guard))
                return invalid;
        }
        else if (!band_power_helpers::filter_direction (result, sections, false))
            return invalid;
        for (double &sample : result)
        {
            sample *= scale;
            if (!std::isfinite (sample))
                return invalid;
        }
        std::copy (result.begin (), result.end (), data);
        return ok;
    }

    struct FftPlan
    {
        kiss_fftr_cfg cfg;
        explicit FftPlan (int n, bool inverse = false) : cfg (kiss_fftr_alloc (n, inverse, 0, 0))
        {
            if (!cfg)
                throw std::bad_alloc ();
        }
        ~FftPlan ()
        {
            kiss_fftr_free (cfg);
        }
        FftPlan (const FftPlan &) = delete;
        FftPlan &operator= (const FftPlan &) = delete;
    };

    int spectrum (double *data, int n, int nfft, int overlap, int fs, int window,
        double *amplitudes, double *frequencies)
    {
        if (!amplitudes || !frequencies || nfft < 2 || nfft % 2 || n < nfft || fs < 1 ||
            overlap < 0 || overlap >= nfft || !finite_signal (data, n))
            return invalid;
        std::vector<double> weights (nfft), input (nfft), result (nfft / 2 + 1, 0.0);
        std::vector<kiss_fft_cpx> output (nfft / 2 + 1);
        int res = get_window (window, nfft, weights.data ());
        if (res != ok)
            return res;
        double energy = 0.0;
        for (double value : weights)
            energy += value * value;
        if (!(energy > 0.0))
            return invalid;
        const double denominator = std::sqrt ((double)fs * energy);
        const int step = nfft - overlap;
        const int count = 1 + (n - nfft) / step;
        FftPlan plan (nfft);
        for (int segment = 0; segment < count; segment++)
        {
            const size_t start = (size_t)segment * step;
            // Normalize before the transform to avoid overflowing an otherwise representable PSD.
            for (int i = 0; i < nfft; i++)
                input[i] = data[start + i] * (weights[i] / denominator);
            kiss_fftr (plan.cfg, input.data (), output.data ());
            for (int i = 0; i <= nfft / 2; i++)
            {
                const double magnitude = std::hypot (output[i].r, output[i].i);
                const double factor = (i == 0 || i == nfft / 2) ? 1.0 : 2.0;
                const double averaged_magnitude = magnitude / std::sqrt ((double)count);
                const double value = averaged_magnitude * averaged_magnitude * factor;
                if (!std::isfinite (value))
                    return invalid;
                result[i] += value;
                if (!std::isfinite (result[i]))
                    return invalid;
            }
        }
        std::copy (result.begin (), result.end (), amplitudes);
        for (int i = 0; i <= nfft / 2; i++)
            frequencies[i] = i * ((double)fs / nfft);
        return ok;
    }

    int parse_file (const char *name, std::vector<double> &values, int &rows, int &cols)
    {
        if (!name)
            return invalid;
        std::ifstream stream (name);
        stream.imbue (std::locale::classic ());
        if (!stream)
            return invalid;
        rows = cols = 0;
        std::string line;
        while (std::getline (stream, line))
        {
            if (!line.empty () && line.back () == '\r')
                line.pop_back ();
            if (line.empty ())
                return invalid;
            const char separator = line.find ('\t') != std::string::npos ? '\t' : ',';
            if (line.back () == separator)
                return invalid;
            std::istringstream row (line);
            std::string token;
            int width = 0;
            while (std::getline (row, token, separator))
            {
                std::istringstream number (token);
                number.imbue (std::locale::classic ());
                double value;
                if (!(number >> value) || !std::isfinite (value))
                    return invalid;
                number >> std::ws;
                if (!number.eof () || values.size () >= (size_t)std::numeric_limits<int>::max ())
                    return invalid;
                values.push_back (value);
                width++;
            }
            if (!width || (cols && cols != width))
                return invalid;
            cols = width;
            rows++;
        }
        return stream.bad () || !rows ? invalid : ok;
    }
}

int perform_lowpass (double *data, int n, int fs, double cutoff, int order, int type, double ripple)
try
{
    return filter_signal (data, n, fs, cutoff, 0, order, type, ripple, 0);
}
catch (...)
{
    return failure;
}

int get_filter_settling_samples (int kind, int fs, double low, double high, int order, int type,
    double ripple, int *edge_samples)
try
{
    if (!edge_samples)
        return invalid;
    std::vector<filter_helpers::Section> sections;
    double radius = 0.0;
    int guard;
    if (!filter_helpers::design (kind, fs, low, high, order, type, ripple, sections, radius) ||
        !filter_helpers::settling_samples (sections, radius, guard))
        return invalid;
    *edge_samples = guard;
    return ok;
}
catch (...)
{
    return failure;
}

int create_streaming_filter (
    int kind, int fs, double low, double high, int order, int type, double ripple, int *handle)
try
{
    if (!handle || type < 0 || type > 2)
        return invalid;
    auto state = std::make_shared<StreamingFilterState> ();
    double radius = 0;
    if (!filter_helpers::design (kind, fs, low, high, order, type, ripple, state->sections, radius))
        return invalid;
    state->z1.resize (state->sections.size (), 0);
    state->z2.resize (state->sections.size (), 0);
    std::lock_guard<std::mutex> lock (streaming_registry_mutex);
    if (next_streaming_handle > std::numeric_limits<int>::max ())
        return failure;
    const int identifier = (int)next_streaming_handle++;
    streaming_filters.emplace (identifier, state);
    *handle = identifier;
    return ok;
}
catch (...)
{
    return failure;
}

int perform_streaming_filter (int handle, double *data, int n)
try
{
    if (!finite_signal (data, n))
        return invalid;
    auto state = find_streaming_filter (handle);
    if (!state)
        return invalid;
    std::lock_guard<std::mutex> lock (state->mutex);
    std::vector<double> result (data, data + n), z1 = state->z1, z2 = state->z2;
    for (size_t i = 0; i < state->sections.size (); i++)
    {
        const auto &s = state->sections[i];
        for (double &sample : result)
        {
            const double filtered = s.b0 * sample + z1[i];
            z1[i] = s.b1 * sample - s.a1 * filtered + z2[i];
            z2[i] = s.b2 * sample - s.a2 * filtered;
            if (!std::isfinite (filtered) || !std::isfinite (z1[i]) || !std::isfinite (z2[i]))
                return invalid;
            sample = filtered;
        }
    }
    state->z1.swap (z1);
    state->z2.swap (z2);
    std::copy (result.begin (), result.end (), data);
    return ok;
}
catch (...)
{
    return failure;
}

int reset_streaming_filter (int handle)
try
{
    auto state = find_streaming_filter (handle);
    if (!state)
        return invalid;
    std::lock_guard<std::mutex> lock (state->mutex);
    std::fill (state->z1.begin (), state->z1.end (), 0.0);
    std::fill (state->z2.begin (), state->z2.end (), 0.0);
    return ok;
}
catch (...)
{
    return failure;
}

int release_streaming_filter (int handle)
try
{
    std::lock_guard<std::mutex> lock (streaming_registry_mutex);
    return streaming_filters.erase (handle) ? ok : invalid;
}
catch (...)
{
    return failure;
}

int perform_decimation (const double *data, int n, int factor, double *output)
try
{
    if (!output || factor < 1 || factor > n || factor > std::numeric_limits<int>::max () / 20 ||
        !finite_signal (data, n))
        return invalid;
    if (factor == 1)
    {
        std::copy (data, data + n, output);
        return ok;
    }
    const int half = 10 * factor;
    const double pi = std::acos (-1.0), cutoff = 0.4 / factor;
    std::vector<double> coefficients (2 * half + 1);
    double sum = 0;
    for (int i = -half; i <= half; i++)
    {
        const double sinc = i == 0 ? 2 * cutoff : std::sin (2 * pi * cutoff * i) / (pi * i);
        coefficients[i + half] = sinc * (0.54 + 0.46 * std::cos (pi * i / half));
        sum += coefficients[i + half];
    }
    const double scale = signal_scale (data, n);
    std::vector<double> result (n / factor, 0.0);
    if (scale > 0)
    {
        const int64_t reflection_period = 2LL * (n - 1);
        for (int j = 0; j < n / factor; j++)
        {
            double value = 0;
            for (int k = -half; k <= half; k++)
            {
                int64_t index = ((int64_t)j * factor + k) % reflection_period;
                if (index < 0)
                    index += reflection_period;
                if (index >= n)
                    index = reflection_period - index;
                value += (data[index] / scale) * (coefficients[k + half] / sum);
            }
            result[j] = value * scale;
            if (!std::isfinite (result[j]))
                return invalid;
        }
    }
    std::copy (result.begin (), result.end (), output);
    return ok;
}
catch (...)
{
    return failure;
}

int perform_highpass (
    double *data, int n, int fs, double cutoff, int order, int type, double ripple)
try
{
    return filter_signal (data, n, fs, cutoff, 0, order, type, ripple, 1);
}
catch (...)
{
    return failure;
}

int perform_bandpass (
    double *data, int n, int fs, double low, double high, int order, int type, double ripple)
try
{
    return filter_signal (data, n, fs, low, high, order, type, ripple, 2);
}
catch (...)
{
    return failure;
}

int perform_bandstop (
    double *data, int n, int fs, double low, double high, int order, int type, double ripple)
try
{
    return filter_signal (data, n, fs, low, high, order, type, ripple, 3);
}
catch (...)
{
    return failure;
}

int remove_environmental_noise (double *data, int n, int fs, int noise_type)
try
{
    if (noise_type < 0 || noise_type > 2 || !finite_signal (data, n) || fs <= 0 ||
        ((noise_type == 0 || noise_type == 2) && fs / 2.0 <= 52) ||
        ((noise_type == 1 || noise_type == 2) && fs / 2.0 <= 62))
        return invalid;
    std::vector<double> result (data, data + n);
    int res = ok;
    if (noise_type == 0 || noise_type == 2)
        res = perform_bandstop (result.data (), n, fs, 48, 52, 4, 3, 0);
    if (res == ok && (noise_type == 1 || noise_type == 2))
        res = perform_bandstop (result.data (), n, fs, 58, 62, 4, 3, 0);
    if (res == ok)
        std::copy (result.begin (), result.end (), data);
    return res;
}
catch (...)
{
    return failure;
}

int perform_fft (double *data, int n, int window, double *re, double *im)
try
{
    if (!re || !im || n < 2 || n % 2 || !finite_signal (data, n))
        return invalid;
    std::vector<double> input (n);
    int res = get_window (window, n, input.data ());
    if (res != ok)
        return res;
    for (int i = 0; i < n; i++)
        input[i] *= data[i];
    std::vector<kiss_fft_cpx> output (n / 2 + 1);
    FftPlan plan (n);
    kiss_fftr (plan.cfg, input.data (), output.data ());
    for (auto value : output)
        if (!std::isfinite (value.r) || !std::isfinite (value.i))
            return invalid;
    for (int i = 0; i <= n / 2; i++)
    {
        re[i] = output[i].r;
        im[i] = output[i].i;
    }
    return ok;
}
catch (...)
{
    return failure;
}

int perform_ifft (double *re, double *im, int n, double *restored)
try
{
    if (!restored || n < 2 || n % 2 || !finite_signal (re, n / 2 + 1) ||
        !finite_signal (im, n / 2 + 1))
        return invalid;
    std::vector<kiss_fft_cpx> input (n / 2 + 1);
    for (int i = 0; i <= n / 2; i++)
    {
        input[i].r = re[i] / n;
        input[i].i = im[i] / n;
    }
    std::vector<double> output (n);
    FftPlan plan (n, true);
    kiss_fftri (plan.cfg, input.data (), output.data ());
    if (!finite_signal (output.data (), n))
        return invalid;
    std::copy (output.begin (), output.end (), restored);
    return ok;
}
catch (...)
{
    return failure;
}

int get_psd (double *data, int n, int fs, int window, double *ampl, double *freq)
try
{
    return spectrum (data, n, n, 0, fs, window, ampl, freq);
}
catch (...)
{
    return failure;
}

int get_psd_welch (
    double *data, int n, int nfft, int overlap, int fs, int window, double *ampl, double *freq)
try
{
    if (nfft < 2 || (nfft & (nfft - 1)))
        return invalid;
    return spectrum (data, n, nfft, overlap, fs, window, ampl, freq);
}
catch (...)
{
    return failure;
}

int get_nearest_power_of_two (int value, int *output)
{
    if (!output || value < 1)
        return invalid;
    int64_t lower = 1;
    while (lower * 2 <= value)
        lower *= 2;
    const int64_t upper = lower * 2;
    const int64_t result = value - lower < upper - value ? lower : upper;
    if (result > std::numeric_limits<int>::max ())
        return invalid;
    *output = (int)result;
    return ok;
}

int calc_stddev (double *data, int start, int end, double *output)
{
    if (!data || !output || start < 0 || end <= start || !finite_signal (data + start, end - start))
        return invalid;
    const double result = stddev (data + start, end - start);
    if (!std::isfinite (result))
        return invalid;
    *output = result;
    return ok;
}

int detrend (double *data, int n, int operation)
try
{
    if (operation < 0 || operation > 2 || !finite_signal (data, n))
        return invalid;
    if (operation == 0)
        return ok;
    const double scale = signal_scale (data, n);
    if (scale == 0.0)
        return ok;
    const double average = mean (data, n) / scale;
    double slope = 0.0;
    if (operation == 2 && n > 1)
    {
        const double center = (n - 1) / 2.0;
        double covariance = 0.0, correction = 0.0;
        for (int i = 0; i < n; i++)
        {
            const double value = ((i - center) / n) * (data[i] / scale - average) / n - correction;
            const double next = covariance + value;
            correction = (next - covariance) - value;
            covariance = next;
        }
        slope = covariance / ((1.0 - 1.0 / ((double)n * n)) / 12.0);
    }
    std::vector<double> result (n);
    for (int i = 0; i < n; i++)
        result[i] = ((data[i] / scale - average) - slope * ((i - (n - 1) / 2.0) / n)) * scale;
    if (!finite_signal (result.data (), n))
        return invalid;
    std::copy (result.begin (), result.end (), data);
    return ok;
}
catch (...)
{
    return failure;
}

int perform_rolling_filter (double *data, int n, int period, int operation)
try
{
    if (period <= 0 || operation < 0 || operation > 2 || !finite_signal (data, n))
        return invalid;
    if (operation == 2)
        return ok;
    const double scale = signal_scale (data, n);
    if (scale == 0.0)
        return ok;
    std::vector<double> original (data, data + n);
    std::multiset<double> lower, upper;
    double sum = 0.0, correction = 0.0;
    auto add_sum = [&] (double value)
    {
        const double adjusted = value - correction;
        const double next = sum + adjusted;
        correction = (next - sum) - adjusted;
        sum = next;
    };
    for (int i = 0; i < n; i++)
    {
        if (operation == 0)
        {
            if (i >= period)
                add_sum (-original[i - period] / scale);
            add_sum (original[i] / scale);
            data[i] = std::max (-1.0, std::min (1.0, sum / std::min (period, i + 1))) * scale;
        }
        else
        {
            if (i >= period)
            {
                auto it = lower.find (original[i - period]);
                if (it != lower.end ())
                    lower.erase (it);
                else
                    upper.erase (upper.find (original[i - period]));
            }
            if (lower.empty () || original[i] <= *lower.rbegin ())
                lower.insert (original[i]);
            else
                upper.insert (original[i]);
            while (lower.size () > upper.size () + 1)
            {
                auto it = std::prev (lower.end ());
                upper.insert (*it);
                lower.erase (it);
            }
            while (lower.size () < upper.size ())
            {
                auto it = upper.begin ();
                lower.insert (*it);
                upper.erase (it);
            }
            // Removing the sole lower element can leave its replacement above upper's minimum.
            if (!upper.empty () && *lower.rbegin () > *upper.begin ())
            {
                auto l = std::prev (lower.end ());
                auto u = upper.begin ();
                double a = *l, b = *u;
                lower.erase (l);
                upper.erase (u);
                lower.insert (b);
                upper.insert (a);
            }
            data[i] = lower.size () == upper.size () ? *lower.rbegin () / 2 + *upper.begin () / 2 :
                                                       *lower.rbegin ();
        }
    }
    return ok;
}
catch (...)
{
    return failure;
}

int perform_downsampling (double *data, int n, int period, int operation, double *output)
try
{
    if (!output || period < 1 || operation < 0 || operation > 2 || !finite_signal (data, n))
        return invalid;
    // No complete block means no output, even when period is much larger than the input.
    if (period > n)
        return ok;
    std::vector<double> block;
    if (operation == 1)
        block.resize (period);
    for (int i = 0; i < n / period; i++)
    {
        const double *start = data + (size_t)i * period;
        if (operation == 0)
            output[i] = mean (start, period);
        else if (operation == 2)
            output[i] = start[period - 1];
        else
        {
            std::copy (start, start + period, block.begin ());
            std::sort (block.begin (), block.end ());
            output[i] =
                period % 2 ? block[period / 2] : block[period / 2 - 1] / 2 + block[period / 2] / 2;
        }
    }
    return ok;
}
catch (...)
{
    return failure;
}

int write_file (const double *data, int rows, int cols, const char *name, const char *mode)
try
{
    if (!data || !name || !mode || rows < 1 || cols < 1 ||
        (int64_t)rows * cols > std::numeric_limits<int>::max () ||
        !finite_signal (data, rows * cols))
        return invalid;
    const std::string selected (mode);
    if (selected != "w" && selected != "w+" && selected != "a" && selected != "a+")
        return invalid;
    std::ofstream stream (
        name, std::ios::out | (selected[0] == 'a' ? std::ios::app : std::ios::trunc));
    stream.imbue (std::locale::classic ());
    if (!stream)
        return invalid;
    stream << std::setprecision (std::numeric_limits<double>::max_digits10);
    for (int i = 0; i < cols; i++)
    {
        for (int j = 0; j < rows; j++)
        {
            if (j)
                stream << '\t';
            stream << data[(size_t)j * cols + i];
        }
        stream << '\n';
    }
    stream.close ();
    return stream ? ok : failure;
}
catch (...)
{
    return failure;
}

int get_num_elements_in_file (const char *name, int *count)
try
{
    if (!count)
        return invalid;
    std::vector<double> values;
    int rows, cols;
    int res = parse_file (name, values, rows, cols);
    if (res == ok)
        *count = (int)values.size ();
    return res;
}
catch (...)
{
    return failure;
}

int read_file (double *data, int *rows, int *cols, const char *name, int capacity)
try
{
    if (!data || !rows || !cols || capacity <= 0)
        return invalid;
    std::vector<double> values;
    int file_rows, file_cols;
    int res = parse_file (name, values, file_rows, file_cols);
    if (res != ok)
        return res;
    if (values.size () > (size_t)capacity)
        return short_buffer;
    for (int i = 0; i < file_rows; i++)
        for (int j = 0; j < file_cols; j++)
            data[(size_t)j * file_rows + i] = values[(size_t)i * file_cols + j];
    *rows = file_cols;
    *cols = file_rows;
    return ok;
}
catch (...)
{
    return failure;
}
