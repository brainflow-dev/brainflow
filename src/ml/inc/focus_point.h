#pragma once

#include <algorithm>
#include <array>


class FocusPoint : public std::array<double, 10>
{
public:
    static const int DIM = 10; // required for kdtree

    int value;

    FocusPoint ()
    {
        value = 0;
        for (int i = 0; i < 10; i++)
        {
            (*this)[i] = 0.0;
        }
    }

    FocusPoint (double *feature_vector, int vector_len, int value)
    {
        this->value = value;
        int len = std::min (vector_len, 10);
        for (int i = 0; i < len; i++)
        {
            (*this)[i] = feature_vector[i];
        }
        // to work without stddev in feature vector if needed
        for (int i = vector_len; i < 10; i++)
        {
            (*this)[i] = 0.0;
        }
    }
};
