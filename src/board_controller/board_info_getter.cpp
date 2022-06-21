#include <fstream>
#include <set>
#include <string.h>
#include <string>
#include <vector>

#include "board.h"
#include "board_info_getter.h"
#include "brainflow_boards.h"
#include "brainflow_constants.h"


static int get_single_value (
    int board_id, const char *preset, const char *param_name, int *value, bool use_logger = true);
static int get_string_value (int board_id, const char *preset, const char *param_name, char *string,
    int *len, bool use_logger = true);
static int get_array_value (int board_id, const char *preset, const char *param_name,
    int *output_array, int *len, bool use_logger = true);


int get_board_presets (int board_id, int max_len, char *presets, int *len)
{
    std::string res = "";
    try
    {
        for (auto &el : brainflow_boards_json["boards"][std::to_string (board_id)].items ())
        {
            res += el.key () + ",";
        }
        if (res.empty ())
        {
            Board::board_logger->error ("no presets found"); // more likely will never be executed
            return (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
        else
        {
            res.pop_back ();
            strncpy (presets, res.c_str (), max_len);
            *len = (int)strlen (res.c_str ());
        }
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    catch (json::exception &e)
    {
        Board::board_logger->error (
            "Failed to get board info: {}, usually it means that you provided wrong board id",
            e.what ());
        return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
    }
}

int get_board_descr (int board_id, const char *preset, char *board_descr, int *len)
{
    std::string preset_str = "default";
    if (preset != NULL)
    {
        preset_str = preset;
    }
    try
    {
        std::string res =
            brainflow_boards_json["boards"][std::to_string (board_id)][preset_str].dump ();
        Board::board_logger->error ("asd {}", res.c_str ());
        if ((res.empty ()) || (res == "null"))
        {
            return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
        }
        else
        {
            strcpy (board_descr, res.c_str ());
            *len = (int)strlen (res.c_str ());
            return (int)BrainFlowExitCodes::STATUS_OK;
        }
    }
    catch (json::exception &e)
    {
        Board::board_logger->error (
            "Failed to get board info: {}, usually it means that you provided wrong board id",
            e.what ());
        return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
    }
}

int get_sampling_rate (int board_id, const char *preset, int *sampling_rate)
{
    return get_single_value (board_id, preset, "sampling_rate", sampling_rate);
}

int get_package_num_channel (int board_id, const char *preset, int *package_num_channel)
{
    return get_single_value (board_id, preset, "package_num_channel", package_num_channel);
}

int get_marker_channel (int board_id, const char *preset, int *marker_channel)
{
    return get_single_value (board_id, preset, "marker_channel", marker_channel);
}

int get_battery_channel (int board_id, const char *preset, int *battery_channel)
{
    return get_single_value (board_id, preset, "battery_channel", battery_channel);
}

int get_num_rows (int board_id, const char *preset, int *num_rows)
{
    return get_single_value (board_id, preset, "num_rows", num_rows);
}

int get_timestamp_channel (int board_id, const char *preset, int *timestamp_channel)
{
    return get_single_value (board_id, preset, "timestamp_channel", timestamp_channel);
}

int get_eeg_names (int board_id, const char *preset, char *eeg_names, int *len)
{
    return get_string_value (board_id, preset, "eeg_names", eeg_names, len);
}

int get_device_name (int board_id, const char *preset, char *name, int *len)
{
    return get_string_value (board_id, preset, "name", name, len);
}

int get_eeg_channels (int board_id, const char *preset, int *eeg_channels, int *len)
{
    return get_array_value (board_id, preset, "eeg_channels", eeg_channels, len);
}

int get_emg_channels (int board_id, const char *preset, int *emg_channels, int *len)
{
    return get_array_value (board_id, preset, "emg_channels", emg_channels, len);
}

int get_ecg_channels (int board_id, const char *preset, int *ecg_channels, int *len)
{
    return get_array_value (board_id, preset, "ecg_channels", ecg_channels, len);
}

int get_eog_channels (int board_id, const char *preset, int *eog_channels, int *len)
{
    return get_array_value (board_id, preset, "eog_channels", eog_channels, len);
}

int get_eda_channels (int board_id, const char *preset, int *eda_channels, int *len)
{
    return get_array_value (board_id, preset, "eda_channels", eda_channels, len);
}

int get_ppg_channels (int board_id, const char *preset, int *ppg_channels, int *len)
{
    return get_array_value (board_id, preset, "ppg_channels", ppg_channels, len);
}

int get_accel_channels (int board_id, const char *preset, int *accel_channels, int *len)
{
    return get_array_value (board_id, preset, "accel_channels", accel_channels, len);
}

int get_analog_channels (int board_id, const char *preset, int *analog_channels, int *len)
{
    return get_array_value (board_id, preset, "analog_channels", analog_channels, len);
}

int get_gyro_channels (int board_id, const char *preset, int *gyro_channels, int *len)
{
    return get_array_value (board_id, preset, "gyro_channels", gyro_channels, len);
}

int get_other_channels (int board_id, const char *preset, int *other_channels, int *len)
{
    return get_array_value (board_id, preset, "other_channels", other_channels, len);
}

int get_temperature_channels (int board_id, const char *preset, int *temperature_channels, int *len)
{
    return get_array_value (board_id, preset, "temperature_channels", temperature_channels, len);
}

int get_resistance_channels (int board_id, const char *preset, int *resistance_channels, int *len)
{
    return get_array_value (board_id, preset, "resistance_channels", resistance_channels, len);
}

int get_exg_channels (int board_id, const char *preset, int *exg_channels, int *len)
{
    std::set<int> unique_channels;
    const char *data_types[4] = {"eeg_channels", "emg_channels", "ecg_channels", "eog_channels"};
    for (int i = 0; i < 4; i++)
    {
        int channels[4096] = {0};
        int channels_len = 0;
        int res = get_array_value (board_id, preset, data_types[i], channels, &channels_len, false);
        if (res == (int)BrainFlowExitCodes::STATUS_OK)
        {
            for (int i = 0; i < channels_len; i++)
            {
                unique_channels.insert (channels[i]);
            }
        }
    }
    int counter = 0;
    for (std::set<int>::iterator it = unique_channels.begin (); it != unique_channels.end (); it++)
    {
        exg_channels[counter] = *it;
        counter++;
    }
    *len = counter;
    if (counter == 0)
    {
        return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

static int get_single_value (
    int board_id, const char *preset, const char *param_name, int *value, bool use_logger)
{
    std::string preset_str = "default";
    if (preset != NULL)
    {
        preset_str = preset;
    }
    try
    {
        int val =
            (int)brainflow_boards_json["boards"][std::to_string (board_id)][preset_str][param_name];
        *value = val;
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    catch (json::exception &e)
    {
        if (use_logger)
        {
            Board::board_logger->error (
                "Failed to get board info: {}, usually it means that device has no such channels, "
                "use get_board_descr method for the info about supported channels",
                e.what ());
        }
        return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
    }
}

static int get_array_value (int board_id, const char *preset, const char *param_name,
    int *output_array, int *len, bool use_logger)
{
    std::string preset_str = "default";
    if (preset != NULL)
    {
        preset_str = preset;
    }
    try
    {
        std::vector<int> values =
            brainflow_boards_json["boards"][std::to_string (board_id)][preset_str][param_name];
        if (!values.empty ())
        {
            memcpy (output_array, &values[0], sizeof (int) * values.size ());
        }
        *len = (int)values.size ();
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    catch (json::exception &e)
    {
        if (use_logger)
        {
            Board::board_logger->error (
                "Failed to get board info: {}, usually it means that device has no such channels, "
                "use get_board_descr method for the info about supported channels",
                e.what ());
        }
        return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
    }
}

static int get_string_value (int board_id, const char *preset, const char *param_name, char *string,
    int *len, bool use_logger)
{
    std::string preset_str = "default";
    if (preset != NULL)
    {
        preset_str = preset;
    }
    try
    {
        std::string val =
            brainflow_boards_json["boards"][std::to_string (board_id)][preset_str][param_name];
        strcpy (string, val.c_str ());
        *len = (int)strlen (val.c_str ());
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    catch (json::exception &e)
    {
        if (use_logger)
        {
            Board::board_logger->error (
                "Failed to get board info: {}, usually it means that device has no such channels, "
                "use get_board_descr method for the info about supported channels",
                e.what ());
        }
        return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
    }
}
