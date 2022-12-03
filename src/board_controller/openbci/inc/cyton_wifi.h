#pragma once

#include <math.h>

#include "openbci_gain_tracker.h"
#include "openbci_wifi_shield_board.h"

class CytonWifi : public OpenBCIWifiShieldBoard
{
protected:
    CytonGainTracker gain_tracker;

    void read_thread ();

public:
    // package num, 8 eeg channels, 3 accel channels
    CytonWifi (struct BrainFlowInputParams params)
        : OpenBCIWifiShieldBoard (params, (int)BoardIds::CYTON_WIFI_BOARD)
    {
    }

    int prepare_session ();
    int config_board (std::string config, std::string &response);
};
