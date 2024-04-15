#pragma once

#include <math.h>
#include <string>

#include "openbci_gain_tracker.h"
#include "openbci_serial_board.h"

class Cyton : public OpenBCISerialBoard
{
protected:
    CytonGainTracker gain_tracker;

    void read_thread ();

public:
    Cyton (struct BrainFlowInputParams params)
        : OpenBCISerialBoard (params, (int)BoardIds::CYTON_BOARD)
    {
    }

    int config_board (std::string config, std::string &response);
};
