#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
// windows sockets version 2 should be added before windows.h(seems like there is a version
// conflict), windows sockets are used for wifi boards
#include <windows.h>
#endif

#include <map>
#include <memory>
#include <mutex>
#include <utility>

#include "board.h"
#include "board_controller.h"
#include "brainbit.h"
#include "brainflow_input_params.h"
#include "callibri_ecg.h"
#include "callibri_eeg.h"
#include "callibri_emg.h"
#include "cyton.h"
#include "cyton_daisy.h"
#include "cyton_daisy_wifi.h"
#include "cyton_wifi.h"
#include "ganglion.h"
#include "ganglion_wifi.h"
#include "novaxr.h"
#include "streaming_board.h"
#include "synthetic_board.h"
#include "unicorn_board.h"

#include "json.hpp"

using json = nlohmann::json;


std::map<std::pair<int, struct BrainFlowInputParams>, std::shared_ptr<Board>> boards;
std::mutex mutex;

std::pair<int, struct BrainFlowInputParams> get_key (
    int board_id, struct BrainFlowInputParams params);
static int check_board_session (int board_id, char *json_brainflow_input_params,
    std::pair<int, struct BrainFlowInputParams> &key, bool log_error = true);
static int string_to_brainflow_input_params (
    const char *json_brainflow_input_params, struct BrainFlowInputParams *params);


int prepare_session (int board_id, char *json_brainflow_input_params)
{
    std::lock_guard<std::mutex> lock (mutex);

    Board::board_logger->info ("incomming json: {}", json_brainflow_input_params);
    struct BrainFlowInputParams params;
    int res = string_to_brainflow_input_params (json_brainflow_input_params, &params);
    if (res != STATUS_OK)
    {
        return res;
    }

    std::pair<int, struct BrainFlowInputParams> key = get_key (board_id, params);
    if (boards.find (key) != boards.end ())
    {
        Board::board_logger->error (
            "Board with id {} and the same config already exists", board_id);
        return ANOTHER_BOARD_IS_CREATED_ERROR;
    }

    std::shared_ptr<Board> board = NULL;
    switch (board_id)
    {
        case STREAMING_BOARD:
            board = std::shared_ptr<Board> (new StreamingBoard (params));
            break;
        case SYNTHETIC_BOARD:
            board = std::shared_ptr<Board> (new SyntheticBoard (params));
            break;
        case CYTON_BOARD:
            board = std::shared_ptr<Board> (new Cyton (params));
            break;
        case GANGLION_BOARD:
            board = std::shared_ptr<Board> (new Ganglion (params));
            break;
        case CYTON_DAISY_BOARD:
            board = std::shared_ptr<Board> (new CytonDaisy (params));
            break;
        case NOVAXR_BOARD:
            board = std::shared_ptr<Board> (new NovaXR (params));
            break;
        case GANGLION_WIFI_BOARD:
            board = std::shared_ptr<Board> (new GanglionWifi (params));
            break;
        case CYTON_WIFI_BOARD:
            board = std::shared_ptr<Board> (new CytonWifi (params));
            break;
        case CYTON_DAISY_WIFI_BOARD:
            board = std::shared_ptr<Board> (new CytonDaisyWifi (params));
            break;
        case BRAINBIT_BOARD:
            board = std::shared_ptr<Board> (new BrainBit (params));
            break;
        case UNICORN_BOARD:
            board = std::shared_ptr<Board> (new UnicornBoard (params));
            break;
        case CALLIBRI_EEG_BOARD:
            board = std::shared_ptr<Board> (new CallibriEEG (params));
            break;
        case CALLIBRI_EMG_BOARD:
            board = std::shared_ptr<Board> (new CallibriEMG (params));
            break;
        case CALLIBRI_ECG_BOARD:
            board = std::shared_ptr<Board> (new CallibriECG (params));
            break;
        default:
            return UNSUPPORTED_BOARD_ERROR;
    }
    res = board->prepare_session ();
    if (res != STATUS_OK)
    {
        board = NULL;
    }
    else
    {
        boards[key] = board;
    }
    return res;
}

int is_prepared (int *prepared, int board_id, char *json_brainflow_input_params)
{
    std::lock_guard<std::mutex> lock (mutex);

    std::pair<int, struct BrainFlowInputParams> key;
    int res = check_board_session (board_id, json_brainflow_input_params, key, false);
    if (res == STATUS_OK)
    {
        *prepared = 1;
    }
    if (res == BOARD_NOT_CREATED_ERROR)
    {
        *prepared = 0;
        res = STATUS_OK;
    }
    return res;
}

int start_stream (
    int buffer_size, char *streamer_params, int board_id, char *json_brainflow_input_params)
{
    std::lock_guard<std::mutex> lock (mutex);

    std::pair<int, struct BrainFlowInputParams> key;
    int res = check_board_session (board_id, json_brainflow_input_params, key, false);
    if (res != STATUS_OK)
    {
        return res;
    }
    auto board_it = boards.find (key);
    return board_it->second->start_stream (buffer_size, streamer_params);
}

int stop_stream (int board_id, char *json_brainflow_input_params)
{
    std::lock_guard<std::mutex> lock (mutex);

    std::pair<int, struct BrainFlowInputParams> key;
    int res = check_board_session (board_id, json_brainflow_input_params, key, false);
    if (res != STATUS_OK)
    {
        return res;
    }
    auto board_it = boards.find (key);
    return board_it->second->stop_stream ();
}

int release_session (int board_id, char *json_brainflow_input_params)
{
    std::lock_guard<std::mutex> lock (mutex);

    std::pair<int, struct BrainFlowInputParams> key;
    int res = check_board_session (board_id, json_brainflow_input_params, key, false);
    if (res != STATUS_OK)
    {
        return res;
    }
    auto board_it = boards.find (key);
    res = board_it->second->release_session ();
    boards.erase (board_it);
    return res;
}

int get_current_board_data (int num_samples, double *data_buf, int *returned_samples, int board_id,
    char *json_brainflow_input_params)
{
    std::lock_guard<std::mutex> lock (mutex);

    std::pair<int, struct BrainFlowInputParams> key;
    int res = check_board_session (board_id, json_brainflow_input_params, key, false);
    if (res != STATUS_OK)
    {
        return res;
    }
    auto board_it = boards.find (key);
    return board_it->second->get_current_board_data (num_samples, data_buf, returned_samples);
}

int get_board_data_count (int *result, int board_id, char *json_brainflow_input_params)
{
    std::lock_guard<std::mutex> lock (mutex);

    std::pair<int, struct BrainFlowInputParams> key;
    int res = check_board_session (board_id, json_brainflow_input_params, key, false);
    if (res != STATUS_OK)
    {
        return res;
    }
    auto board_it = boards.find (key);
    return board_it->second->get_board_data_count (result);
}

int get_board_data (
    int data_count, double *data_buf, int board_id, char *json_brainflow_input_params)
{
    std::lock_guard<std::mutex> lock (mutex);

    std::pair<int, struct BrainFlowInputParams> key;
    int res = check_board_session (board_id, json_brainflow_input_params, key, false);
    if (res != STATUS_OK)
    {
        return res;
    }
    auto board_it = boards.find (key);
    return board_it->second->get_board_data (data_count, data_buf);
}

int set_log_level (int log_level)
{
    std::lock_guard<std::mutex> lock (mutex);
    return Board::set_log_level (log_level);
}

int log_message (int log_level, char *log_message)
{
    // its a method for loggging from high level api dont add it to Board class since it should not
    // be used internally
    std::lock_guard<std::mutex> lock (mutex);
    int level;
    if (log_level < 0)
    {
        Board::board_logger->warn ("log level should be >= 0");
        level = 0;
    }
    else if (log_level > 6)
    {
        Board::board_logger->warn ("log level should be <= 6");
        level = 6;
    }
    else
    {
        level = log_level;
    }

    Board::board_logger->log (spdlog::level::level_enum (log_level), "{}", log_message);

    return STATUS_OK;
}

int set_log_file (char *log_file)
{
    std::lock_guard<std::mutex> lock (mutex);
    return Board::set_log_file (log_file);
}

int config_board (char *config, int board_id, char *json_brainflow_input_params)
{
    std::lock_guard<std::mutex> lock (mutex);

    std::pair<int, struct BrainFlowInputParams> key;
    int res = check_board_session (board_id, json_brainflow_input_params, key, false);
    if (res != STATUS_OK)
    {
        return res;
    }
    auto board_it = boards.find (key);
    return board_it->second->config_board (config);
}

/////////////////////////////////////////////////
//////////////////// helpers ////////////////////
/////////////////////////////////////////////////

std::pair<int, struct BrainFlowInputParams> get_key (
    int board_id, struct BrainFlowInputParams params)
{
    std::pair<int, struct BrainFlowInputParams> key = std::make_pair (board_id, params);
    return key;
}

int check_board_session (int board_id, char *json_brainflow_input_params,
    std::pair<int, struct BrainFlowInputParams> &key, bool log_error)
{
    struct BrainFlowInputParams params;
    int res = string_to_brainflow_input_params (json_brainflow_input_params, &params);
    if (res != STATUS_OK)
    {
        return res;
    }

    key = get_key (board_id, params);

    if (boards.find (key) == boards.end ())
    {
        if (log_error)
        {
            Board::board_logger->error (
                "Board with id {} and port provided config is not created", key.first);
        }
        return BOARD_NOT_CREATED_ERROR;
    }
    return STATUS_OK;
}

int string_to_brainflow_input_params (
    const char *json_brainflow_input_params, struct BrainFlowInputParams *params)
{
    // input string -> json -> struct BrainFlowInputParams
    try
    {
        json config = json::parse (std::string (json_brainflow_input_params));
        params->serial_port = config["serial_port"];
        params->ip_protocol = config["ip_protocol"];
        params->ip_port = config["ip_port"];
        params->other_info = config["other_info"];
        params->mac_address = config["mac_address"];
        params->ip_address = config["ip_address"];
        params->timeout = config["timeout"];
        params->serial_number = config["serial_number"];
        return STATUS_OK;
    }
    catch (json::exception &e)
    {
        Board::board_logger->error ("invalid input json, {}", e.what ());
        return GENERAL_ERROR;
    }
}
