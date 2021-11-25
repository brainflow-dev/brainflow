#pragma once

#include "dyn_lib_classifier.h"


class ONNXClassifier : public DynLibClassifier
{
public:
    ONNXClassifier (struct BrainFlowModelParams params);
    virtual ~ONNXClassifier ();

protected:
    static bool env_set;

    virtual std::string get_dyn_lib_path ();
};
