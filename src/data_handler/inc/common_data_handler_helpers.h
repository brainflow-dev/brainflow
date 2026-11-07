#pragma once

#include <algorithm>
#include <cmath>
#include <limits>

inline bool finite_signal (const double *data, int n)
{
    if (!data || n <= 0)
        return false;
    for (int i = 0; i < n; i++)
        if (!std::isfinite (data[i]))
            return false;
    return true;
}

inline double signal_scale (const double *data, int n)
{
    double scale = 0.0;
    for (int i = 0; i < n; i++)
        scale = std::max (scale, std::abs (data[i]));
    return scale;
}

inline double rms (double x[], int n)
{
    const double scale = signal_scale (x, n);
    if (scale == 0.0)
        return 0.0;
    double sum = 0;
    for (int i = 0; i < n; i++)
    {
        const double value = x[i] / scale;
        sum += value * value / n;
    }
    return scale * sqrt (std::min (1.0, sum));
}

inline double mean (const double x[], int n)
{
    const double scale = signal_scale (x, n);
    if (scale == 0.0)
        return 0.0;
    double sum = 0, correction = 0;
    for (int i = 0; i < n; i++)
    {
        const double value = (x[i] / scale) / n - correction;
        const double next = sum + value;
        correction = (next - sum) - value;
        sum = next;
    }
    return std::max (-1.0, std::min (1.0, sum)) * scale;
}

// The caller validates the finite input and operation. Input and output may be
// identical: all trend parameters are computed before any sample is written.
inline bool detrend_signal (const double *data, int n, int operation, double scale, double *result)
{
    if (operation == 0 || scale == 0.0)
    {
        if (result != data)
            std::copy (data, data + n, result);
        return true;
    }
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
    for (int i = 0; i < n; i++)
        result[i] = ((data[i] / scale - average) - slope * ((i - (n - 1) / 2.0) / n)) * scale;
    return finite_signal (result, n);
}

inline double stddev (const double data[], int len)
{
    const double scale = signal_scale (data, len);
    if (scale == 0.0)
        return 0.0;
    double the_mean = mean (data, len) / scale;
    double deviation = 0.0;

    for (int i = 0; i < len; ++i)
    {
        const double delta = data[i] / scale - the_mean;
        deviation += delta * delta / len;
    }

    return scale * sqrt (std::min (1.0, deviation));
}


// Reverses the drection of an array of doubles.
// I.e. the first element (0) is replaced by the last (len - 1),
// element 1 is replaced by element (len - 1 - 1), etc.
inline void reverse_array (double data[], int len)
{
    for (int i = 0; i < len / 2; i++)
    {
        double temp = data[i];
        data[i] = data[len - i - 1];
        data[len - i - 1] = temp;
    }
}
