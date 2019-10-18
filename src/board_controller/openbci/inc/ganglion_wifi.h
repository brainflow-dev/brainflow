#pragma once

#include <math.h>

#include "openbci_wifi_shield_board.h"

#define ADS1299_Vref 4.5
#define ADS1299_gain 24.0

class GanglionWifi : public OpenBCIWifiShieldBoard
{
    float const accel_scale = 0.032f;
    float const eeg_scale = (1.2f * 1000000) / (8388607.0f * 1.5f * 51.0f);

protected:
    void read_thread ();

public:
    // package num, 4 eeg channels, 3 accel channels
    GanglionWifi (struct BrainFlowInputParams params)
        : OpenBCIWifiShieldBoard (8, params, (int)GANGLION_WIFI_BOARD)
    {
    }
};
