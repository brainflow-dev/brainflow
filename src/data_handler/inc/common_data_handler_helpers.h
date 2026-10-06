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
