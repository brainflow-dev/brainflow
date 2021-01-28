#pragma once

#include <math.h>

#include "openbci_serial_board.h"

#define ADS1299_Vref 4.5
#define ADS1299_gain 24.0

class CytonDaisy : public OpenBCISerialBoard
{
    double eeg_scale = (double)(ADS1299_Vref / float ((pow (2, 23) - 1)) / ADS1299_gain * 1000000.);
    double accel_scale = (double)(0.002 / (pow (2, 4)));

protected:
    void read_thread ();

public:
    CytonDaisy (struct BrainFlowInputParams params)
        : OpenBCISerialBoard (params, (int)BoardIds::CYTON_DAISY_BOARD)
    {
    }
};
