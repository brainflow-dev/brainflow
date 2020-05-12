#pragma once

#include <math.h>

#include "openbci_wifi_shield_board.h"

#define ADS1299_Vref 4.5
#define ADS1299_gain 24.0

class GanglionWifi : public OpenBCIWifiShieldBoard
{
    float const accel_scale = (0.02f / 16.0f) / 1.25f; // 1.25 experimental number
    float const eeg_scale = (1.2f * 1000000) / (8388607.0f * 1.5f * 51.0f);

    volatile bool is_cheking_impedance;

protected:
    void read_thread ();
    void read_thread_impedance ();

public:
    // package num, 4 eeg channels, 3 accel channels
    GanglionWifi (struct BrainFlowInputParams params)
        : OpenBCIWifiShieldBoard (23, params, (int)GANGLION_WIFI_BOARD)
    {
        is_cheking_impedance = false;
    }

    // hacks for ganglion and impedance
    int start_stream (int buffer_size, char *streamer_params);
    int stop_stream ();
    int config_board (char *config);
};
