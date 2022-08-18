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
