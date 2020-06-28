#pragma once

#include "callibri.h"

class CallibriECG : public Callibri
{

#if defined _WIN32 || defined __APPLE__

protected:
    int apply_initial_settings ();
#endif

public:
    CallibriECG (struct BrainFlowInputParams params)
        : Callibri ((int)BoardIds::CALLIBRI_ECG_BOARD, params)
    {
    }
    ~CallibriECG ()
    {
    }
};
