#pragma once

#include <math.h>
#include <string>

#include "expandable_eeg32_wifi_gain_tracker.h"
#include "openbci_wifi_shield_board.h"

class ExpandableEeg32Wifi : public OpenBCIWifiShieldBoard
{
protected:
    ExpandableEeg32WifiGainTracker gain_tracker;

    void read_thread ();

    /** Map user symbols to wire protocol if needed; currently passthrough. */
    static std::string translate_user_config_to_device (const std::string &conf);

public:
    // 32 EEG rows per merged packet; see brainflow_boards ExpandableEeg32Wifi
    ExpandableEeg32Wifi (struct BrainFlowInputParams params)
        : OpenBCIWifiShieldBoard (params, (int)BoardIds::EXPANDABLE_EEG_32_WIFI_BOARD)
    {
    }

    int prepare_session ();
    int config_board (std::string config, std::string &response);
};
