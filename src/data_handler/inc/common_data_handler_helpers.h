#pragma once

#include <math.h>
#include <stdlib.h>

inline double rms (double x[], int n)
{
    double sum = 0;
    for (int i = 0; i < n; i++)
    {
        sum += x[i] * x[i];
    }
    return sqrt (sum / n);
}

inline double mean (double x[], int n)
{
    double sum = 0;
    for (int i = 0; i < n; i++)
    {
        sum += x[i];
    }
    return sum / n;
}

inline double stddev (double data[], int len)
{
    double the_mean = mean (data, len);
    double deviation = 0.0;

    for (int i = 0; i < len; ++i)
    {
        deviation += pow (data[i] - the_mean, 2);
    }

    return sqrt (deviation / len);
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