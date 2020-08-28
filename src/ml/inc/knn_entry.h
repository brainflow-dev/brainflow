#pragma once

#include <algorithm>
#include <math.h>
#include <string.h>


class KNNEntry
{
public:
    double *feature_vector;
    int value;
    int vector_len;
    double distance;

    KNNEntry (double *feature_vector, int value, int vector_len)
    {
        this->value = value;
        this->vector_len = vector_len;
        this->feature_vector = new double[vector_len];
        memcpy (this->feature_vector, feature_vector, sizeof (double) * vector_len);
        distance = 0.0;
    }

    KNNEntry (const KNNEntry &other)
    {
        value = other.value;
        vector_len = other.vector_len;
        feature_vector = new double[other.vector_len];
        memcpy (feature_vector, other.feature_vector, sizeof (double) * vector_len);
        distance = other.distance;
    }

    ~KNNEntry ()
    {
        delete[] feature_vector;
    }

    bool operator< (const KNNEntry &other) const
    {
        return distance < other.distance;
    }

    bool operator> (const KNNEntry &other) const
    {
        return distance > other.distance;
    }

    double set_distance (const KNNEntry &other)
    {
        int size = std::min (vector_len, other.vector_len);
        double distance = 0.0;
        for (int i = 0; i < size; i++)
        {
            distance += (feature_vector[i] - other.feature_vector[i]) *
                (feature_vector[i] - other.feature_vector[i]);
        }
        return sqrt (distance);
    }
};
