#pragma once

#include <math.h>

#include "openbci_wifi_shield_board.h"

#define ADS1299_Vref 4.5
#define ADS1299_gain 24.0

class CytonDaisyWifi : public OpenBCIWifiShieldBoard
{
    float eeg_scale = ADS1299_Vref / float((pow (2, 23) - 1)) / ADS1299_gain * 1000000.;
    float accel_scale = 0.002 / (pow (2, 4));

protected:
    void read_thread ();

public:
    // package num, 16 eeg channels, 3 accel channels
    CytonDaisyWifi (struct BrainFlowInputParams params)
        : OpenBCIWifiShieldBoard (30, params, (int)CYTON_DAISY_WIFI_BOARD)
    {
    }

    int prepare_session ();
};
