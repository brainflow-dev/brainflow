#pragma once

#include "callibri.h"

class CallibriEMG : public Callibri
{

#if defined _WIN32 || defined __APPLE__

protected:
    int apply_initial_settings ();
#endif

public:
    CallibriEMG (const struct BrainFlowInputParams &params)
        : Callibri ((int)BoardIds::CALLIBRI_EMG_BOARD, params)
    {
    }
    ~CallibriEMG ()
    {
    }
};
