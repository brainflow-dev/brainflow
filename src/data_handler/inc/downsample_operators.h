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
    std::vector<double> values;
    for (int i = 0; i < len; i++)
    {
        values.push_back (data[i]);
    }
    std::sort (values.begin (), values.end ());
    if (len % 2 == 0)
    {
        // for an even number of values the median is the mean of the two middle ones,
        // same convention as RollingMedian in rolling_filter.h
        return (values[len / 2 - 1] + values[len / 2]) / 2.0;
    }
    return values[len / 2];
}
