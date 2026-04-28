#pragma once

#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>

#include "brainflow_constants.h"


namespace MuseOptions
{
    enum class PresetFamily
    {
        Legacy,
        Anthena
    };

    inline std::string trim_string (const std::string &value)
    {
        size_t first = value.find_first_not_of (" \t\r\n");
        if (first == std::string::npos)
        {
            return "";
        }
        size_t last = value.find_last_not_of (" \t\r\n");
        return value.substr (first, last - first + 1);
    }

    inline std::string to_lower (const std::string &value)
    {
        std::string lower_value = value;
        std::transform (lower_value.begin (), lower_value.end (), lower_value.begin (),
            [] (unsigned char ch) { return (char)std::tolower (ch); });
        return lower_value;
    }

    inline bool parse_bool_option (const std::string &value, bool &parsed)
    {
        std::string lower_value = to_lower (trim_string (value));
        if (lower_value == "true")
        {
            parsed = true;
            return true;
        }
        if (lower_value == "false")
        {
            parsed = false;
            return true;
        }
        return false;
    }

    inline bool is_valid_anthena_preset (const std::string &preset)
    {
        static const char *valid_presets[] = {"p20", "p21", "p50", "p51", "p60", "p61", "p1034",
            "p1035", "p1041", "p1042", "p1043", "p1044", "p1045", "p1046", "p4129"};

        for (size_t i = 0; i < sizeof (valid_presets) / sizeof (valid_presets[0]); i++)
        {
            if (preset == valid_presets[i])
            {
                return true;
            }
        }
        return false;
    }

    inline bool is_valid_legacy_preset (int board_id, const std::string &preset)
    {
        if ((preset == "p20") || (preset == "p21"))
        {
            return true;
        }

        bool is_muse_2016 = (board_id == (int)BoardIds::MUSE_2016_BOARD) ||
            (board_id == (int)BoardIds::MUSE_2016_BLED_BOARD);
        if ((preset == "p50") || (preset == "p51"))
        {
            return !is_muse_2016;
        }

        bool is_muse_s = (board_id == (int)BoardIds::MUSE_S_BOARD) ||
            (board_id == (int)BoardIds::MUSE_S_BLED_BOARD);
        if ((preset == "p60") || (preset == "p61"))
        {
            return is_muse_s;
        }

        return false;
    }

    inline bool is_valid_preset (int board_id, PresetFamily family, const std::string &preset)
    {
        if (family == PresetFamily::Anthena)
        {
            return is_valid_anthena_preset (preset);
        }
        return is_valid_legacy_preset (board_id, preset);
    }

    inline bool parse_preset_options (const std::string &other_info, int board_id,
        PresetFamily family, bool allow_low_latency, std::string &preset, bool &enable_low_latency,
        std::string &error)
    {
        std::string trimmed = trim_string (other_info);
        if (trimmed.empty ())
        {
            return true;
        }

        if ((trimmed.find ('=') == std::string::npos) && (trimmed.find (';') == std::string::npos))
        {
            std::string parsed_preset = to_lower (trimmed);
            if (!is_valid_preset (board_id, family, parsed_preset))
            {
                error = "invalid Muse preset: " + trimmed;
                return false;
            }
            preset = parsed_preset;
            return true;
        }

        bool has_options = false;
        std::stringstream ss (trimmed);
        std::string token;
        while (std::getline (ss, token, ';'))
        {
            token = trim_string (token);
            if (token.empty ())
            {
                continue;
            }

            size_t separator = token.find ('=');
            if ((separator == std::string::npos) ||
                (token.find ('=', separator + 1) != std::string::npos))
            {
                error = "invalid Muse other_info option: " + token + ". Expected key=value";
                return false;
            }

            std::string key = to_lower (trim_string (token.substr (0, separator)));
            std::string value = trim_string (token.substr (separator + 1));
            if (key.empty () || value.empty ())
            {
                error = "invalid Muse other_info option: " + token + ". Empty key or value";
                return false;
            }

            if (key == "preset")
            {
                value = to_lower (value);
                if (!is_valid_preset (board_id, family, value))
                {
                    error = "invalid Muse preset: " + value;
                    return false;
                }
                preset = value;
                has_options = true;
            }
            else if ((key == "low_latency") && allow_low_latency)
            {
                bool parsed = false;
                if (!parse_bool_option (value, parsed))
                {
                    error = "invalid Muse low_latency value: " + value;
                    return false;
                }
                enable_low_latency = parsed;
                has_options = true;
            }
            else
            {
                error = "unknown Muse other_info option: " + key;
                return false;
            }
        }

        if (!has_options)
        {
            error = "empty Muse other_info";
            return false;
        }

        return true;
    }
} // namespace MuseOptions
