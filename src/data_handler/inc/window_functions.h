#pragma once

#include <algorithm>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

inline double no_window_function (double *data, int data_len)
{
    double windowed_data[data_len];
    for (int i = 0; i < data_len; i++)
    {
        windowed_data[i] = data[i];
    }
    return windowed_data;
}

inline double hanning_function (double data, int data_len)
{
    double windowed_data[data_len];
    for (int i = 0; i < data_len; i++)
    {
        windowed_data[i] = data[i] * (0.5 - 0.5 * cos (2.0 * M_PI * i / data_len));
    }
    return windowed_data;
}

inline double hamming_function (double *data, int data_len)
{
    double windowed_data[data_len];
    for (int i = 0; i < data_len; i++)
    {
        windowed_data[i] = data[i] * (0.54 - 0.46 * cos (2.0 * M_PI * i / data_len));
    }
    return windowed_data;
}

inline double blackman_harris_function (double *data, int data_len)
{
    double windowed_data[data_len];
    for (int i = 0; i < data_len; i++)
    {
        windowed_data[i] = data[i] *
                            (0.355768 - 0.487396 * cos (2.0 * M_PI * i / data_len) +
                            0.144232 * cos (4.0 * M_PI * i / data_len) -
                            0.012604 * cos (6.0 * M_PI * i / data_len));
    }
    return windowed_data;
}
