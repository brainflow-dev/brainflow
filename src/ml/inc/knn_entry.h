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

    KNNEntry ()
    {
        feature_vector = NULL;
        value = 0;
        vector_len = 0;
        distance = 0.0;
    }

    KNNEntry (double *feature_vector, int value, int vector_len)
    {
        this->value = value;
        this->vector_len = vector_len;
        this->feature_vector = new double[vector_len];
        for (int i = 0; i < vector_len; i++)
        {
            this->feature_vector[i] = feature_vector[i];
        }
        distance = 0.0;
    }

    KNNEntry (const KNNEntry &other)
    {
        value = other.value;
        vector_len = other.vector_len;
        if (feature_vector != NULL)
        {
            delete[] feature_vector;
            feature_vector = NULL;
        }
        if (other.feature_vector != NULL)
        {
            feature_vector = new double[other.vector_len];
            for (int i = 0; i < other.vector_len; i++)
            {
                feature_vector[i] = other.feature_vector[i];
            }
        }
        else
        {
            feature_vector = NULL;
        }
        distance = other.distance;
    }

    KNNEntry &operator= (const KNNEntry &other)
    {
        if (this != &other)
        {
            if (feature_vector != NULL)
            {
                delete[] feature_vector;
                feature_vector = NULL;
            }
            value = other.value;
            vector_len = other.vector_len;
            if (other.feature_vector != NULL)
            {
                feature_vector = new double[other.vector_len];
                for (int i = 0; i < other.vector_len; i++)
                {
                    feature_vector[i] = other.feature_vector[i];
                }
            }
            else
            {
                feature_vector = NULL;
            }
            distance = other.distance;
        }
        return *this;
    }

    ~KNNEntry ()
    {
        if (feature_vector != NULL)
        {
            delete[] feature_vector;
            feature_vector = NULL;
        }
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
