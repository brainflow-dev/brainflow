#include <fstream>
#include <sstream>
#include <string.h>
#include <string>
#include <vector>

#include "board_info_getter.h"
#include "brainflow_constants.h"
#include "get_dll_dir.h"

#include "json.hpp"

#define JSON_FILE_NAME "brainflow_boards.json"

using json = nlohmann::json;


inline std::string get_json_config ();
inline bool check_file_exists (const std::string &name);
inline std::string int_to_string (int val);
inline int get_single_value (int board_id, const char *param_name, int *value);
inline int get_string_value (int board_id, const char *param_name, char *string, int *len);
inline int get_array_value (int board_id, const char *param_name, int *output_array, int *len);


int get_sampling_rate (int board_id, int *sampling_rate)
{
    return get_single_value (board_id, "sampling_rate", sampling_rate);
}

int get_package_num_channel (int board_id, int *package_num_channel)
{
    return get_single_value (board_id, "package_num_channel", package_num_channel);
}

int get_battery_channel (int board_id, int *battery_channel)
{
    return get_single_value (board_id, "battery_channel", battery_channel);
}

int get_num_rows (int board_id, int *num_rows)
{
    return get_single_value (board_id, "num_rows", num_rows);
}

int get_timestamp_channel (int board_id, int *timestamp_channel)
{
    return get_single_value (board_id, "timestamp_channel", timestamp_channel);
}

int get_eeg_names (int board_id, char *eeg_names, int *len)
{
    return get_string_value (board_id, "eeg_names", eeg_names, len);
}

int get_eeg_channels (int board_id, int *eeg_channels, int *len)
{
    return get_array_value (board_id, "eeg_channels", eeg_channels, len);
}

int get_emg_channels (int board_id, int *emg_channels, int *len)
{
    return get_array_value (board_id, "emg_channels", emg_channels, len);
}

int get_ecg_channels (int board_id, int *ecg_channels, int *len)
{
    return get_array_value (board_id, "ecg_channels", ecg_channels, len);
}

int get_eog_channels (int board_id, int *eog_channels, int *len)
{
    return get_array_value (board_id, "eog_channels", eog_channels, len);
}

int get_eda_channels (int board_id, int *eda_channels, int *len)
{
    return get_array_value (board_id, "eda_channels", eda_channels, len);
}

int get_ppg_channels (int board_id, int *ppg_channels, int *len)
{
    return get_array_value (board_id, "ppg_channels", ppg_channels, len);
}

int get_accel_channels (int board_id, int *accel_channels, int *len)
{
    return get_array_value (board_id, "accel_channels", accel_channels, len);
}

int get_analog_channels (int board_id, int *analog_channels, int *len)
{
    return get_array_value (board_id, "analog_channels", analog_channels, len);
}

int get_gyro_channels (int board_id, int *gyro_channels, int *len)
{
    return get_array_value (board_id, "gyro_channels", gyro_channels, len);
}

int get_other_channels (int board_id, int *other_channels, int *len)
{
    return get_array_value (board_id, "other_channels", other_channels, len);
}

int get_temperature_channels (int board_id, int *temperature_channels, int *len)
{
    return get_array_value (board_id, "temperature_channels", temperature_channels, len);
}

int get_resistance_channels (int board_id, int *resistance_channels, int *len)
{
    return get_array_value (board_id, "resistance_channels", resistance_channels, len);
}

inline std::string get_json_config ()
{
    char lib_dir[1024];
    bool res = get_dll_path (lib_dir);
    std::string json_path = "";
    if (res)
    {
        json_path = std::string (lib_dir) + JSON_FILE_NAME;
    }
    else
    {
        json_path = JSON_FILE_NAME;
    }
    return json_path;
}

inline bool check_file_exists (const std::string &name)
{
    if (FILE *file = fopen (name.c_str (), "r"))
    {
        fclose (file);
        return true;
    }
    else
    {
        return false;
    }
}

inline std::string int_to_string (int val)
{
    std::ostringstream ss;
    ss << val;
    return ss.str ();
}

inline int get_single_value (int board_id, const char *param_name, int *value)
{
    std::string json_file = get_json_config ();
    if (!check_file_exists (json_file))
    {
        return JSON_NOT_FOUND_ERROR;
    }
    try
    {
        std::ifstream json_stream (json_file);
        json config = json::parse (json_stream);
        int val = (int)config["boards"][int_to_string (board_id)][param_name];
        *value = val;
        return STATUS_OK;
    }
    catch (json::exception &e)
    {
        return NO_SUCH_DATA_IN_JSON_ERROR;
    }
}

inline int get_array_value (int board_id, const char *param_name, int *output_array, int *len)
{
    std::string json_file = get_json_config ();
    if (!check_file_exists (json_file))
    {
        return JSON_NOT_FOUND_ERROR;
    }
    try
    {
        std::ifstream json_stream (json_file);
        json config = json::parse (json_stream);
        std::vector<int> values = config["boards"][int_to_string (board_id)][param_name];
        if (!values.empty ())
        {
            memcpy (output_array, &values[0], sizeof (int) * values.size ());
        }
        *len = values.size ();
        return STATUS_OK;
    }
    catch (json::exception &e)
    {
        return UNSUPPORTED_BOARD_ERROR;
    }
}

inline int get_string_value (int board_id, const char *param_name, char *string, int *len)
{
    std::string json_file = get_json_config ();
    if (!check_file_exists (json_file))
    {
        return JSON_NOT_FOUND_ERROR;
    }
    try
    {
        std::ifstream json_stream (json_file);
        json config = json::parse (json_stream);
        std::string val = config["boards"][int_to_string (board_id)][param_name];
        strcpy (string, val.c_str ());
        *len = strlen (val.c_str ());
        return STATUS_OK;
    }
    catch (json::exception &e)
    {
        return NO_SUCH_DATA_IN_JSON_ERROR;
    }
}