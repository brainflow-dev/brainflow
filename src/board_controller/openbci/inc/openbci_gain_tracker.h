#pragma once

#include <algorithm>
#include <stdlib.h>
#include <string>
#include <vector>


enum class OpenBCICommandTypes : int
{
    NOT_CHANNEL_COMMAND = 0,
    VALID_COMMAND = 1,
    INVALID_COMMAND = 2
};


class OpenBCIGainTracker
{
protected:
    size_t single_command_size = 9;
    std::vector<char> channel_letters;
    std::vector<int> current_gains;
    std::vector<int> old_gains;
    std::vector<int> available_gain_values;

    int apply_single_command (std::string command)
    {
        // start stop validation
        if ((command.size () < single_command_size) || (command.at (0) != 'x') ||
            (command.at (single_command_size - 1) != 'X'))
        {
            return (int)OpenBCICommandTypes::NOT_CHANNEL_COMMAND;
        }
        // bias srb1 srb2 validation
        if ((command.at (5) != '0') && (command.at (5) != '1') ||
            (command.at (6) != '0') && (command.at (6) != '1') ||
            (command.at (7) != '0') && (command.at (7) != '1'))
        {
            return (int)OpenBCICommandTypes::INVALID_COMMAND;
        }
        // input type check
        if ((command.at (4) < '0') || (command.at (4) > '7'))
        {
            return (int)OpenBCICommandTypes::INVALID_COMMAND;
        }
        // gain check
        if ((command.at (3) < '0') || (command.at (3) > '6'))
        {
            return (int)OpenBCICommandTypes::INVALID_COMMAND;
        }
        // power check
        if ((command.at (2) != '0') && (command.at (2) != '1'))
        {
            return (int)OpenBCICommandTypes::INVALID_COMMAND;
        }
        // channel check
        auto channel_it =
            std::find (channel_letters.begin (), channel_letters.end (), command.at (1));
        if (channel_it == channel_letters.end ())
        {
            return (int)OpenBCICommandTypes::INVALID_COMMAND;
        }
        size_t index = std::distance (channel_letters.begin (), channel_it);
        if (index >= current_gains.size ())
        {
            return (int)OpenBCICommandTypes::INVALID_COMMAND;
        }
        old_gains[index] = current_gains[index];
        current_gains[index] = available_gain_values[command.at (3) - '0'];
        return (int)OpenBCICommandTypes::VALID_COMMAND;
    }

public:
    OpenBCIGainTracker (std::vector<int> default_gains)
        : current_gains (default_gains), old_gains (default_gains)
    {
        channel_letters = std::vector<char> {
            '1', '2', '3', '4', '5', '6', '7', '8', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I'};
        available_gain_values = std::vector<int> {1, 2, 4, 6, 8, 12, 24};
    }

    virtual ~OpenBCIGainTracker ()
    {
    }

    virtual int apply_config (std::string config)
    {
        // x (CHANNEL, POWER_DOWN, GAIN_SET, INPUT_TYPE_SET, BIAS_SET, SRB2_SET, SRB1_SET) X
        // https://docs.openbci.com/Cyton/CytonSDK/

        int res = (int)OpenBCICommandTypes::NOT_CHANNEL_COMMAND;

        for (size_t i = 0; i < config.size ();)
        {
            if (config.at (i) == 'x')
            {
                if ((config.size () >= i + single_command_size) &&
                    (config.at (i + single_command_size - 1) == 'X'))
                {
                    res = apply_single_command (config.substr (i, single_command_size));
                    i += single_command_size;
                }
                else
                {
                    i++;
                }
            }
            else
            {
                i++;
            }
        }
        return res;
    }

    virtual int get_gain_for_channel (int channel)
    {
        if (channel > (int)current_gains.size ())
        {
            return 1; // should never happen
        }
        return current_gains[channel];
    }

    virtual void revert_config ()
    {
        std::copy (old_gains.begin (), old_gains.end (), current_gains.begin ());
    }
};

class CytonGainTracker : public OpenBCIGainTracker
{
public:
    CytonGainTracker () : OpenBCIGainTracker ({24, 24, 24, 24, 24, 24, 24, 24})
    {
    }

    virtual int apply_config (std::string config)
    {
        if (config.size () == 1)
        {
            // restore default settings
            if (config.at (0) == 'd')
            {
                std::copy (current_gains.begin (), current_gains.end (), old_gains.begin ());
                std::fill (current_gains.begin (), current_gains.end (), 24);
            }
        }

        return OpenBCIGainTracker::apply_config (config);
    }
};

class CytonDaisyGainTracker : public OpenBCIGainTracker
{
public:
    CytonDaisyGainTracker ()
        : OpenBCIGainTracker ({24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24})
    {
    }

    virtual int apply_config (std::string config)
    {
        if (config.size () == 1)
        {
            // restore default settings
            if (config.at (0) == 'd')
            {
                std::copy (current_gains.begin (), current_gains.end (), old_gains.begin ());
                std::fill (current_gains.begin (), current_gains.end (), 24);
            }
        }

        return OpenBCIGainTracker::apply_config (config);
    }
};

class GaleaGainTracker : public OpenBCIGainTracker
{
public:
    GaleaGainTracker ()
        : OpenBCIGainTracker ({4, 4, 4, 4, 4, 4, 12, 12, 2, 2, 2, 2, 2, 2, 2, 2}) // to be confirmed
    {
    }

    virtual int apply_config (std::string config)
    {
        if (config.size () == 1)
        {
            if ((config.at (0) == 'f') || (config.at (0) == 'g'))
            {
                std::copy (current_gains.begin (), current_gains.end (), old_gains.begin ());
                std::fill (current_gains.begin (), current_gains.end (), 1);
            }
            if ((config.at (0) == 'o') || (config.at (0) == 'd'))
            {
                std::copy (current_gains.begin (), current_gains.end (), old_gains.begin ());
                for (size_t i = 0; i < current_gains.size (); i++)
                {
                    if (i < 6)
                    {
                        current_gains[i] = 4;
                    }
                    else if ((i == 6) || (i == 7))
                    {
                        current_gains[i] = 12;
                    }
                    else
                    {
                        current_gains[i] = 2;
                    }
                }
            }
        }

        return OpenBCIGainTracker::apply_config (config);
    }
};

class GaleaV4GainTracker : public OpenBCIGainTracker
{
public:
    GaleaV4GainTracker ()
        : OpenBCIGainTracker ({4, 4, 4, 4, 4, 4, 4, 4, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
              12, 12, 12, 12, 12}) // to be confirmed
    {
        channel_letters = std::vector<char> {'1', '2', '3', '4', '5', '6', '7', '8', 'Q', 'W', 'E',
            'R', 'T', 'Y', 'U', 'I', 'A', 'S', 'D', 'G', 'H', 'J', 'K', 'L'};
    }

    virtual int apply_config (std::string config)
    {
        if (config.size () == 1)
        {
            if ((config.at (0) == 'f') || (config.at (0) == 'g'))
            {
                std::copy (current_gains.begin (), current_gains.end (), old_gains.begin ());
                std::fill (current_gains.begin (), current_gains.end (), 1);
            }
            if ((config.at (0) == 'o') || (config.at (0) == 'd'))
            {
                std::copy (current_gains.begin (), current_gains.end (), old_gains.begin ());
                for (size_t i = 0; i < current_gains.size (); i++)
                {
                    if (i < 8)
                    {
                        current_gains[i] = 4;
                    }
                    else
                    {
                        current_gains[i] = 12;
                    }
                }
            }
        }

        return OpenBCIGainTracker::apply_config (config);
    }
};