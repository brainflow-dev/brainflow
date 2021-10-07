#pragma once

#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

inline void no_window_function (int window_len, double *wind)
{
    for (int i = 0; i < window_len; i++)
    {
        wind[i] = 1.0;
    }
}

inline void hanning_function (int window_len, double *wind)
{
    for (int i = 0; i < window_len; i++)
    {
        wind[i] = 0.5 - 0.5 * std::cos (2.0 * M_PI * i / window_len);
    }
}

inline void hamming_function (int window_len, double *wind)
{
    for (int i = 0; i < window_len; i++)
    {
        wind[i] = 0.54 - 0.46 * std::cos (2.0 * M_PI * i / window_len);
    }
}

inline void blackman_harris_function (int window_len, double *wind)
{
    for (int i = 0; i < window_len; i++)
    {
        wind[i] = 0.355768 - 0.487396 * std::cos (2.0 * M_PI * i / window_len) +
            0.144232 * std::cos (4.0 * M_PI * i / window_len) -
            0.012604 * std::cos (6.0 * M_PI * i / window_len);
    }
}
