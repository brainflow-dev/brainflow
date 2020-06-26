#pragma once

#include "callibri.h"

class CallibriEEG : public Callibri
{

#if defined _WIN32 || defined __APPLE__

protected:
    int apply_initial_settings ();
#endif

public:
    CallibriEEG (struct BrainFlowInputParams params)
        : Callibri ((int)BoardIds::CALLIBRI_EEG_BOARD, params)
    {
    }
    ~CallibriEEG ()
    {
    }
};
