#pragma once

#include <algorithm>
#include <vector>

#include "openbci_gain_tracker.h"

/**
 * Gain / x-command channel letters for EXPANDABLE_EEG_32_BOARD and
 * EXPANDABLE_EEG_32_WIFI_BOARD.
 *
 * Ch 1-16: same Cyton+Daisy letters as OpenBCIGainTracker (1-8, QWERTYUI for on / impedance;
 *          !@#$%^& and qwertyui for single-char off on the wire per OpenBCI docs).
 * Ch 17-32: A S D F G H J K Z X C V B N M L (on / impedance); a s d f g h j k z x c v b n m l
 * (off).
 */
class ExpandableEeg32GainTracker : public OpenBCIGainTracker
{
public:
    ExpandableEeg32GainTracker () : OpenBCIGainTracker (std::vector<int> (32, 24))
    {
        channel_letters = std::vector<char> {'1', '2', '3', '4', '5', '6', '7', '8', 'Q', 'W', 'E',
            'R', 'T', 'Y', 'U', 'I', 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'Z', 'X', 'C', 'V',
            'B', 'N', 'M', 'L'};
    }

    int apply_config (std::string config) override
    {
        if (config.size () == 1)
        {
            if (config.at (0) == 'd')
            {
                std::copy (current_gains.begin (), current_gains.end (), old_gains.begin ());
                std::fill (current_gains.begin (), current_gains.end (), 24);
            }
        }

        return OpenBCIGainTracker::apply_config (config);
    }
};

using ExpandableEeg32WifiGainTracker = ExpandableEeg32GainTracker;
