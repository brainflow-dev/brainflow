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
    j["ip_address"] = params.ip_address;
    j["mac_address"] = params.mac_address;
    j["other_info"] = params.other_info;
    j["timeout"] = params.timeout;
    j["serial_number"] = params.serial_number;
    j["file"] = params.file;
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

int BoardShim::get_board_data_count ()
{
    int data_count = 0;
    int res = ::get_board_data_count (&data_count, board_id, serialized_params.c_str ());
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to get board data count", res);
    }
    return data_count;
}

BrainFlowArray<double, 2> BoardShim::get_board_data ()
{
    return get_board_data (get_board_data_count ());
}

BrainFlowArray<double, 2> BoardShim::get_board_data (int num_datapoints)
{
    if (num_datapoints < 0)
    {
        throw BrainFlowException (
            "invalid num_datapoints", (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR);
    }
    int num_samples = std::min (get_board_data_count (), num_datapoints);
    int num_data_channels = get_num_rows (get_board_id ());
    double *buf = new double[num_samples * num_data_channels];
    int res = ::get_board_data (num_samples, buf, board_id, serialized_params.c_str ());
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        delete[] buf;
        throw BrainFlowException ("failed to get board data", res);
    }
    BrainFlowArray<double, 2> matrix (buf, num_data_channels, num_samples);
    delete[] buf;
    return matrix;
}

BrainFlowArray<double, 2> BoardShim::get_current_board_data (int num_samples)
{
    int num_data_channels = BoardShim::get_num_rows (get_board_id ());
    double *buf = new double[num_samples * num_data_channels];
    int len = 0;
    int res =
        ::get_current_board_data (num_samples, buf, &len, board_id, serialized_params.c_str ());
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        delete[] buf;
        throw BrainFlowException ("failed to get board data", res);
    }
    BrainFlowArray<double, 2> matrix (buf, num_data_channels, len);
    delete[] buf;
    return matrix;
}

std::string BoardShim::config_board (char *config)
{
    int response_len = 0;
    char response[8192];
    int res =
        ::config_board (config, response, &response_len, board_id, serialized_params.c_str ());
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to config board", res);
    }
    std::string resp ((const char *)response, response_len);
    return resp;
}

void BoardShim::insert_marker (double value)
{
    int res = ::insert_marker (value, board_id, serialized_params.c_str ());
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
        try
        {
            master_board_id = std::stoi (params.other_info);
        }
        catch (...)
        {
            throw BrainFlowException ("specify master board id using params.other_info",
                (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR);
        }
    }
    return master_board_id;
}

//////////////////////////////////////////
///////////// data desc methods //////////
//////////////////////////////////////////

json BoardShim::get_board_descr (int board_id)
{
    char board_descr_str[16000];
    int string_len = 0;
    int res = ::get_board_descr (board_id, board_descr_str, &string_len);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to get board info", res);
    }
    std::string data (board_descr_str, 0, string_len);
    return json::parse (data);
}

int BoardShim::get_sampling_rate (int board_id)
{
    int sampling_rate = -1;
    int res = ::get_sampling_rate (board_id, &sampling_rate);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to get board info", res);
    }
    return sampling_rate;
}

int BoardShim::get_package_num_channel (int board_id)
{
    int package_num_channel = -1;
    int res = ::get_package_num_channel (board_id, &package_num_channel);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to get board info", res);
    }
    return package_num_channel;
}

int BoardShim::get_timestamp_channel (int board_id)
{
    int timestamp_channel = 0;
    int res = ::get_timestamp_channel (board_id, &timestamp_channel);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to get board info", res);
    }
    return timestamp_channel;
}

int BoardShim::get_marker_channel (int board_id)
{
    int marker_channel = 0;
    int res = ::get_marker_channel (board_id, &marker_channel);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to get board info", res);
    }
    return marker_channel;
}

int BoardShim::get_battery_channel (int board_id)
{
    int battery_channel = 0;
    int res = ::get_battery_channel (board_id, &battery_channel);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to get board info", res);
    }
    return battery_channel;
}

int BoardShim::get_num_rows (int board_id)
{
    int num_rows = 0;
    int res = ::get_num_rows (board_id, &num_rows);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to get board info", res);
    }
    return num_rows;
}

std::vector<std::string> BoardShim::get_eeg_names (int board_id)
{
    char eeg_names[4096];
    int string_len = 0;
    int res = ::get_eeg_names (board_id, eeg_names, &string_len);
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

std::string BoardShim::get_device_name (int board_id)
{
    char name[4096];
    int string_len = 0;
    int res = ::get_device_name (board_id, name, &string_len);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to get board info", res);
    }
    std::string result (name, 0, string_len);
    return result;
}

std::vector<int> BoardShim::get_eeg_channels (int board_id)
{
    int channels[MAX_CHANNELS];
    int len = 0;
    int res = ::get_eeg_channels (board_id, channels, &len);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to get board info", res);
    }
    return std::vector<int> (channels, channels + len);
}

std::vector<int> BoardShim::get_exg_channels (int board_id)
{
    int channels[MAX_CHANNELS];
    int len = 0;
    int res = ::get_exg_channels (board_id, channels, &len);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to get board info", res);
    }
    return std::vector<int> (channels, channels + len);
}

std::vector<int> BoardShim::get_emg_channels (int board_id)
{
    int channels[MAX_CHANNELS];
    int len = 0;
    int res = ::get_emg_channels (board_id, channels, &len);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to get board info", res);
    }
    return std::vector<int> (channels, channels + len);
}

std::vector<int> BoardShim::get_ecg_channels (int board_id)
{
    int channels[MAX_CHANNELS];
    int len = 0;
    int res = ::get_ecg_channels (board_id, channels, &len);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to get board info", res);
    }
    return std::vector<int> (channels, channels + len);
}

std::vector<int> BoardShim::get_eog_channels (int board_id)
{
    int channels[MAX_CHANNELS];
    int len = 0;
    int res = ::get_eog_channels (board_id, channels, &len);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to get board info", res);
    }
    return std::vector<int> (channels, channels + len);
}

std::vector<int> BoardShim::get_eda_channels (int board_id)
{
    int channels[MAX_CHANNELS];
    int len = 0;
    int res = ::get_eda_channels (board_id, channels, &len);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to get board info", res);
    }
    return std::vector<int> (channels, channels + len);
}

std::vector<int> BoardShim::get_ppg_channels (int board_id)
{
    int channels[MAX_CHANNELS];
    int len = 0;
    int res = ::get_ppg_channels (board_id, channels, &len);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to get board info", res);
    }
    return std::vector<int> (channels, channels + len);
}

std::vector<int> BoardShim::get_accel_channels (int board_id)
{
    int channels[MAX_CHANNELS];
    int len = 0;
    int res = ::get_accel_channels (board_id, channels, &len);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to get board info", res);
    }
    return std::vector<int> (channels, channels + len);
}

std::vector<int> BoardShim::get_gyro_channels (int board_id)
{
    int channels[MAX_CHANNELS];
    int len = 0;
    int res = ::get_gyro_channels (board_id, channels, &len);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to get board info", res);
    }
    return std::vector<int> (channels, channels + len);
}

std::vector<int> BoardShim::get_analog_channels (int board_id)
{
    int channels[MAX_CHANNELS];
    int len = 0;
    int res = ::get_analog_channels (board_id, channels, &len);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to get board info", res);
    }
    return std::vector<int> (channels, channels + len);
}

std::vector<int> BoardShim::get_other_channels (int board_id)
{
    int channels[MAX_CHANNELS];
    int len = 0;
    int res = ::get_other_channels (board_id, channels, &len);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to get board info", res);
    }
    return std::vector<int> (channels, channels + len);
}

std::vector<int> BoardShim::get_temperature_channels (int board_id)
{
    int channels[MAX_CHANNELS];
    int len = 0;
    int res = ::get_temperature_channels (board_id, channels, &len);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to get board info", res);
    }
    return std::vector<int> (channels, channels + len);
}

std::vector<int> BoardShim::get_resistance_channels (int board_id)
{
    int channels[MAX_CHANNELS];
    int len = 0;
    int res = ::get_resistance_channels (board_id, channels, &len);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to get board info", res);
    }
    return std::vector<int> (channels, channels + len);
}
