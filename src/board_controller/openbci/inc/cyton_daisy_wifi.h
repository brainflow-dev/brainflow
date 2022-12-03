#pragma once

#include <math.h>
#include <string>

#include "openbci_gain_tracker.h"
#include "openbci_wifi_shield_board.h"

class CytonDaisyWifi : public OpenBCIWifiShieldBoard
{
protected:
    CytonDaisyGainTracker gain_tracker;

    void read_thread ();

public:
    // package num, 16 eeg channels, 3 accel channels
    CytonDaisyWifi (struct BrainFlowInputParams params)
        : OpenBCIWifiShieldBoard (params, (int)BoardIds::CYTON_DAISY_WIFI_BOARD)
    {
    }

    int prepare_session ();
    int config_board (std::string config, std::string &response);
};
