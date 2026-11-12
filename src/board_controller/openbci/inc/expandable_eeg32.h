#pragma once

#include <math.h>
#include <string>
#include <vector>

#include "expandable_eeg32_wifi_gain_tracker.h"
#include "openbci_serial_board.h"

class ExpandableEeg32 : public OpenBCISerialBoard
{
protected:
    ExpandableEeg32GainTracker gain_tracker;

    void read_thread ();

public:
    ExpandableEeg32 (struct BrainFlowInputParams params)
        : OpenBCISerialBoard (params, (int)BoardIds::EXPANDABLE_EEG_32_BOARD)
    {
    }

    int config_board (std::string config, std::string &response);
};
