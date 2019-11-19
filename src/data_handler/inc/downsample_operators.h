#pragma once

#include <algorithm>
#include <vector>

inline double downsample_mean (double *data, int len)
{
    double sum = 0;
    for (int i = 0; i < len; i++)
    {
        sum += data[i];
    }
    return sum / (double)len;
}

inline double downsample_each (double *data, int len)
{
    return data[len - 1];
}

inline double downsample_median (double *data, int len)
{
    if (len % 2 == 0)
    {
        return downsample_mean (data, len);
    }
    std::vector<double> values;
    for (int i = 0; i < len; i++)
    {
        values.push_back (data[i]);
    }
    std::sort (values.begin (), values.end ());
    return values[len / 2];
}
