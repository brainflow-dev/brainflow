#pragma once

#include "base_classifier.h"


class ConcentrationAlgoClassifier : public BaseClassifier
{
public:
    ConcentrationAlgoClassifier ()
    {
    }

    virtual ~ConcentrationAlgoClassifier ()
    {
        release ();
    }

    virtual int prepare ();
    virtual int predict (double *data, int data_len, double *output);
    virtual int release ();
};
