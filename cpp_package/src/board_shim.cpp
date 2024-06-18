#include <algorithm>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

#include "board_shim.h"

#define MAX_CHANNELS 512

/////////////////////////////////////////
/////// serialize struct to json ////////
/////////////////////////////////////////

std::string params_to_string (struct BrainFlowInputParams params)
{
    json j;
    j["serial_port"] = params.serial_port;
    j["ip_protocol"] = params.ip_protocol;
    j["ip_port"] = params.ip_port;
    j["ip_port_aux"] = params.ip_port_aux;
    j["ip_port_anc"] = params.ip_port_anc;
    j["ip_address"] = params.ip_address;
    j["ip_address_aux"] = params.ip_address_aux;
    j["ip_address_anc"] = params.ip_address_anc;
    j["mac_address"] = params.mac_address;
    j["other_info"] = params.other_info;
    j["timeout"] = params.timeout;
    j["serial_number"] = params.serial_number;
    j["file"] = params.file;
    j["file_aux"] = params.file_aux;
    j["file_anc"] = params.file_anc;
    j["master_board"] = params.master_board;
    std::string post_str = j.dump ();
    return post_str;
}

/////////////////////////////////////////
//////////// logging methods ////////////
/////////////////////////////////////////

void BoardShim::enable_board_logger ()
{
    BoardShim::set_log_level ((int)LogLevels::LEVEL_INFO);
}

void BoardShim::disable_board_logger ()
{
    BoardShim::set_log_level ((int)LogLevels::LEVEL_OFF);
}

void BoardShim::enable_dev_board_logger ()
{
    BoardShim::set_log_level ((int)LogLevels::LEVEL_TRACE);
}

void BoardShim::set_log_file (std::string log_file)
{
    int res = set_log_file_board_controller (log_file.c_str ());
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to set log file", res);
    }
}

void BoardShim::set_log_level (int log_level)
{
    int res = set_log_level_board_controller (log_level);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to set log level", res);
    }
}

void BoardShim::log_message (int log_level, const char *format, ...)
{
    char buffer[1024];
    va_list ap;
    va_start (ap, format);
    vsnprintf (buffer, 1024, format, ap);
    va_end (ap);

    int res = log_message_board_controller (log_level, buffer);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to write log message", res);
    }
}

//////////////////////////////////////////
/////// data acquisition methods /////////
//////////////////////////////////////////

void BoardShim::release_all_sessions ()
{
    int res = ::release_all_sessions ();
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to release sessions", res);
    }
}

BoardShim::BoardShim (int board_id, struct BrainFlowInputParams params)
{
    serialized_params = params_to_string (params);
    this->params = params;
    this->board_id = board_id;
}

void BoardShim::prepare_session ()
{
    int res = ::prepare_session (board_id, serialized_params.c_str ());
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to prepare session", res);
    }
}

bool BoardShim::is_prepared ()
{
    int prepared = 0;
    int res = ::is_prepared (&prepared, board_id, serialized_params.c_str ());
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to check session", res);
    }
    return (bool)prepared;
}

void BoardShim::add_streamer (std::string streamer_params, int preset)
{
    int res =
        ::add_streamer (streamer_params.c_str (), preset, board_id, serialized_params.c_str ());
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to add streamer", res);
    }
}

void BoardShim::delete_streamer (std::string streamer_params, int preset)
{
    int res =
        ::delete_streamer (streamer_params.c_str (), preset, board_id, serialized_params.c_str ());
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to delete streamer", res);
    }
}

void BoardShim::start_stream (int buffer_size, std::string streamer_params)
{
    int res = ::start_stream (
        buffer_size, streamer_params.c_str (), board_id, serialized_params.c_str ());
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to start stream", res);
    }
}

void BoardShim::stop_stream ()
{
    int res = ::stop_stream (board_id, serialized_params.c_str ());
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to stop stream", res);
    }
}

void BoardShim::release_session ()
{
    int res = ::release_session (board_id, serialized_params.c_str ());
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to release session", res);
    }
}

int BoardShim::get_board_data_count (int preset)
{
    int data_count = 0;
    int res = ::get_board_data_count (preset, &data_count, board_id, serialized_params.c_str ());
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to get board data count", res);
    }
    return data_count;
}

BrainFlowArray<double, 2> BoardShim::get_board_data (int preset)
{
    return get_board_data (get_board_data_count (preset), preset);
}

BrainFlowArray<double, 2> BoardShim::get_board_data (int num_datapoints, int preset)
{
    if (num_datapoints < 0)
    {
        throw BrainFlowException (
            "invalid num_datapoints", (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR);
    }
    int num_samples = std::min (get_board_data_count (preset), num_datapoints);
    int num_data_channels = get_num_rows (get_board_id (), preset);
    double *buf = new double[num_samples * num_data_channels];
    int res = ::get_board_data (num_samples, preset, buf, board_id, serialized_params.c_str ());
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        delete[] buf;
        throw BrainFlowException ("failed to get board data", res);
    }
    BrainFlowArray<double, 2> matrix (buf, num_data_channels, num_samples);
    delete[] buf;
    return matrix;
}

BrainFlowArray<double, 2> BoardShim::get_current_board_data (int num_samples, int preset)
{
    int num_data_channels = BoardShim::get_num_rows (get_board_id (), preset);
    double *buf = new double[num_samples * num_data_channels];
    int len = 0;
    int res = ::get_current_board_data (
        num_samples, preset, buf, &len, board_id, serialized_params.c_str ());
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        delete[] buf;
        throw BrainFlowException ("failed to get board data", res);
    }
    BrainFlowArray<double, 2> matrix (buf, num_data_channels, len);
    delete[] buf;
    return matrix;
}

std::string BoardShim::config_board (std::string config)
{
    int response_len = 0;
    char response[8192];
    int res = ::config_board (
        config.c_str (), response, &response_len, board_id, serialized_params.c_str ());
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to config board", res);
    }
    std::string resp ((const char *)response, response_len);
    return resp;
}

void BoardShim::config_board_with_bytes (const char *bytes, int len)
{
    int res = ::config_board_with_bytes (bytes, len, board_id, serialized_params.c_str ());
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to config board with bytes", res);
    }
}

void BoardShim::insert_marker (double value, int preset)
{
    int res = ::insert_marker (value, preset, board_id, serialized_params.c_str ());
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to insert marker", res);
    }
}

int BoardShim::get_board_id ()
{
    int master_board_id = board_id;
    if ((board_id == (int)BoardIds::STREAMING_BOARD) ||
        (board_id == (int)BoardIds::PLAYBACK_FILE_BOARD))
    {
        if (params.master_board == (int)BoardIds::NO_BOARD)
        {
            throw BrainFlowException ("specify master board id using params.master_board",
                (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR);
        }
        else
        {
            master_board_id = params.master_board;
        }
    }
    return master_board_id;
}

//////////////////////////////////////////
///////////// data desc methods //////////
//////////////////////////////////////////

json BoardShim::get_board_descr (int board_id, int preset)
{
    char board_descr_str[16000];
    int string_len = 0;
    int res = ::get_board_descr (board_id, preset, board_descr_str, &string_len);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to get board info", res);
    }
    std::string data (board_descr_str, 0, string_len);
    return json::parse (data);
}

int BoardShim::get_sampling_rate (int board_id, int preset)
{
    int sampling_rate = -1;
    int res = ::get_sampling_rate (board_id, preset, &sampling_rate);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to get board info", res);
    }
    return sampling_rate;
}

int BoardShim::get_package_num_channel (int board_id, int preset)
{
    int package_num_channel = -1;
    int res = ::get_package_num_channel (board_id, preset, &package_num_channel);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to get board info", res);
    }
    return package_num_channel;
}

int BoardShim::get_timestamp_channel (int board_id, int preset)
{
    int timestamp_channel = 0;
    int res = ::get_timestamp_channel (board_id, preset, &timestamp_channel);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to get board info", res);
    }
    return timestamp_channel;
}

int BoardShim::get_marker_channel (int board_id, int preset)
{
    int marker_channel = 0;
    int res = ::get_marker_channel (board_id, preset, &marker_channel);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to get board info", res);
    }
    return marker_channel;
}

int BoardShim::get_battery_channel (int board_id, int preset)
{
    int battery_channel = 0;
    int res = ::get_battery_channel (board_id, preset, &battery_channel);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to get board info", res);
    }
    return battery_channel;
}

int BoardShim::get_num_rows (int board_id, int preset)
{
    int num_rows = 0;
    int res = ::get_num_rows (board_id, preset, &num_rows);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to get board info", res);
    }
    return num_rows;
}

std::vector<std::string> BoardShim::get_eeg_names (int board_id, int preset)
{
    char eeg_names[4096];
    int string_len = 0;
    int res = ::get_eeg_names (board_id, preset, eeg_names, &string_len);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to get board info", res);
    }
    std::string line (eeg_names, 0, string_len);
    std::istringstream ss (line);
    std::vector<std::string> out;
    std::string single_name;
    while (std::getline (ss, single_name, ','))
    {
        out.push_back (single_name);
    }

    return out;
}

std::vector<int> BoardShim::get_board_presets (int board_id)
{
    int presets[512];
    int len = 0;
    int res = ::get_board_presets (board_id, presets, &len);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to get board info", res);
    }
    return std::vector<int> (presets, presets + len);
}

std::string BoardShim::get_device_name (int board_id, int preset)
{
    char name[4096];
    int string_len = 0;
    int res = ::get_device_name (board_id, preset, name, &string_len);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to get board info", res);
    }
    std::string result (name, 0, string_len);
    return result;
}

std::vector<int> BoardShim::get_eeg_channels (int board_id, int preset)
{
    int channels[MAX_CHANNELS];
    int len = 0;
    int res = ::get_eeg_channels (board_id, preset, channels, &len);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to get board info", res);
    }
    return std::vector<int> (channels, channels + len);
}

std::vector<int> BoardShim::get_exg_channels (int board_id, int preset)
{
    int channels[MAX_CHANNELS];
    int len = 0;
    int res = ::get_exg_channels (board_id, preset, channels, &len);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to get board info", res);
    }
    return std::vector<int> (channels, channels + len);
}

std::vector<int> BoardShim::get_emg_channels (int board_id, int preset)
{
    int channels[MAX_CHANNELS];
    int len = 0;
    int res = ::get_emg_channels (board_id, preset, channels, &len);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to get board info", res);
    }
    return std::vector<int> (channels, channels + len);
}

std::vector<int> BoardShim::get_ecg_channels (int board_id, int preset)
{
    int channels[MAX_CHANNELS];
    int len = 0;
    int res = ::get_ecg_channels (board_id, preset, channels, &len);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to get board info", res);
    }
    return std::vector<int> (channels, channels + len);
}

std::vector<int> BoardShim::get_eog_channels (int board_id, int preset)
{
    int channels[MAX_CHANNELS];
    int len = 0;
    int res = ::get_eog_channels (board_id, preset, channels, &len);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to get board info", res);
    }
    return std::vector<int> (channels, channels + len);
}

std::vector<int> BoardShim::get_eda_channels (int board_id, int preset)
{
    int channels[MAX_CHANNELS];
    int len = 0;
    int res = ::get_eda_channels (board_id, preset, channels, &len);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to get board info", res);
    }
    return std::vector<int> (channels, channels + len);
}

std::vector<int> BoardShim::get_ppg_channels (int board_id, int preset)
{
    int channels[MAX_CHANNELS];
    int len = 0;
    int res = ::get_ppg_channels (board_id, preset, channels, &len);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to get board info", res);
    }
    return std::vector<int> (channels, channels + len);
}

std::vector<int> BoardShim::get_accel_channels (int board_id, int preset)
{
    int channels[MAX_CHANNELS];
    int len = 0;
    int res = ::get_accel_channels (board_id, preset, channels, &len);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to get board info", res);
    }
    return std::vector<int> (channels, channels + len);
}

std::vector<int> BoardShim::get_rotation_channels (int board_id, int preset)
{
    int channels[MAX_CHANNELS];
    int len = 0;
    int res = ::get_rotation_channels (board_id, preset, channels, &len);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to get board info", res);
    }
    return std::vector<int> (channels, channels + len);
}

std::vector<int> BoardShim::get_gyro_channels (int board_id, int preset)
{
    int channels[MAX_CHANNELS];
    int len = 0;
    int res = ::get_gyro_channels (board_id, preset, channels, &len);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to get board info", res);
    }
    return std::vector<int> (channels, channels + len);
}

std::vector<int> BoardShim::get_analog_channels (int board_id, int preset)
{
    int channels[MAX_CHANNELS];
    int len = 0;
    int res = ::get_analog_channels (board_id, preset, channels, &len);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to get board info", res);
    }
    return std::vector<int> (channels, channels + len);
}

std::vector<int> BoardShim::get_other_channels (int board_id, int preset)
{
    int channels[MAX_CHANNELS];
    int len = 0;
    int res = ::get_other_channels (board_id, preset, channels, &len);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to get board info", res);
    }
    return std::vector<int> (channels, channels + len);
}

std::vector<int> BoardShim::get_temperature_channels (int board_id, int preset)
{
    int channels[MAX_CHANNELS];
    int len = 0;
    int res = ::get_temperature_channels (board_id, preset, channels, &len);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to get board info", res);
    }
    return std::vector<int> (channels, channels + len);
}

std::vector<int> BoardShim::get_resistance_channels (int board_id, int preset)
{
    int channels[MAX_CHANNELS];
    int len = 0;
    int res = ::get_resistance_channels (board_id, preset, channels, &len);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to get board info", res);
    }
    return std::vector<int> (channels, channels + len);
}

std::vector<int> BoardShim::get_magnetometer_channels (int board_id, int preset)
{
    int channels[MAX_CHANNELS];
    int len = 0;
    int res = ::get_magnetometer_channels (board_id, preset, channels, &len);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to get board info", res);
    }
    return std::vector<int> (channels, channels + len);
}

std::string BoardShim::get_version ()
{
    char version[64];
    int string_len = 0;
    int res = ::get_version_board_controller (version, &string_len, 64);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to get board info", res);
    }
    std::string verion_str (version, string_len);

    return verion_str;
}