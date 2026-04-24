#pragma once

#include <string>

#include "brainflow_constants.h"

inline int get_openbci_sampling_rate_for_command (int board_id, char command)
{
    switch (static_cast<BoardIds> (board_id))
    {
        case BoardIds::CYTON_BOARD:
        case BoardIds::CYTON_DAISY_BOARD:
        case BoardIds::CYTON_WIFI_BOARD:
        case BoardIds::CYTON_DAISY_WIFI_BOARD:
        case BoardIds::GALEA_BOARD:
            switch (command)
            {
                case '0':
                    return 16000;
                case '1':
                    return 8000;
                case '2':
                    return 4000;
                case '3':
                    return 2000;
                case '4':
                    return 1000;
                case '5':
                    return 500;
                case '6':
                    return 250;
                default:
                    return -1;
            }
        case BoardIds::GANGLION_BOARD:
        case BoardIds::GANGLION_WIFI_BOARD:
        case BoardIds::GANGLION_NATIVE_BOARD:
            switch (command)
            {
                case '0':
                    return 25600;
                case '1':
                    return 12800;
                case '2':
                    return 6400;
                case '3':
                    return 3200;
                case '4':
                    return 1600;
                case '5':
                    return 800;
                case '6':
                    return 400;
                case '7':
                    return 200;
                default:
                    return -1;
            }
        default:
            return -1;
    }
}

inline bool track_openbci_sampling_rate (int board_id, const std::string &config, int &sampling_rate)
{
    for (size_t i = 0; i + 1 < config.size (); i++)
    {
        if (config[i] != '~')
        {
            continue;
        }
        int rate = get_openbci_sampling_rate_for_command (board_id, config[i + 1]);
        if (rate > 0)
        {
            sampling_rate = rate;
            return true;
        }
    }
    return false;
}
