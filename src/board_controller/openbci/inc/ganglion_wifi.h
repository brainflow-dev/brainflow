#pragma once

#include <math.h>

#include "openbci_wifi_shield_board.h"

#define ADS1299_Vref 4.5
#define ADS1299_gain 24.0

class GanglionWifi : public OpenBCIWifiShieldBoard
{
    double const accel_scale = (0.02 / 16.0) / 1.25; // 1.25 experimental number
    double const eeg_scale = (1.2 * 1000000) / (8388607.0 * 1.5 * 51.0);

    volatile bool is_cheking_impedance;

protected:
    void read_thread ();
    void read_thread_impedance ();

public:
    // package num, 4 eeg channels, 3 accel channels
    GanglionWifi (struct BrainFlowInputParams params)
        : OpenBCIWifiShieldBoard (params, (int)BoardIds::GANGLION_WIFI_BOARD)
    {
        is_cheking_impedance = false;
    }

    // hacks for ganglion and impedance
    int start_stream (int buffer_size, const char *streamer_params);
    int stop_stream ();
    int config_board (std::string config, std::string &response);
};
