#pragma once

#include "brainflow_constants.h"
#include "concentration_lda_classifier.h"


class RelaxationLDAClassifier : public ConcentrationLDAClassifier
{
public:
    RelaxationLDAClassifier (struct BrainFlowModelParams params)
        : ConcentrationLDAClassifier (params)
    {
    }

    int predict (double *data, int data_len, double *output)
    {
        int res = ConcentrationLDAClassifier::predict (data, data_len, output);
        if (res != (int)BrainFlowExitCodes::STATUS_OK)
        {
            return res;
        }
        *output = 1.0 - (*output);
        return res;
    }
};
