#include <sstream>
#include <string.h>

#include "board_info_getter.h"
#include "brainflow_env_vars.h"
#include "streaming_board.h"

#ifndef _WIN32
#include <errno.h>
#endif


StreamingBoard::StreamingBoard (struct BrainFlowInputParams params)
    : Board ((int)BoardIds::STREAMING_BOARD,
          params) // its a hack - set board_id for streaming board here temporary and override it
                  // with master board id in prepare_session, board_id is protected and there is no
                  // api to get it so its ok
{
    keep_alive = false;
    initialized = false;
}

StreamingBoard::~StreamingBoard ()
{
    skip_logs = true;
    release_session ();
}

int StreamingBoard::prepare_session ()
{
    if (initialized)
    {
        safe_logger (spdlog::level::info, "Session is already prepared");
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    if (params.master_board == (int)BoardIds::NO_BOARD)
    {
        safe_logger (spdlog::level::err, "Master board id is not provided");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    try
    {
        board_id = params.master_board;
        board_descr = boards_struct.brainflow_boards_json["boards"][std::to_string (board_id)];
    }
    catch (json::exception &e)
    {
        safe_logger (spdlog::level::err, "Invalid json for master board");
        safe_logger (spdlog::level::err, e.what ());
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    catch (const std::exception &e)
    {
        safe_logger (spdlog::level::err,
            "Write board id for the board which streams data to other_info field");
        safe_logger (spdlog::level::err, e.what ());
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    // default preset
    if ((!params.ip_address.empty ()) && (params.ip_port != 0))
    {
        MultiCastClient *client = new MultiCastClient (params.ip_address.c_str (), params.ip_port);
        clients.push_back (client);
        presets.push_back ((int)BrainFlowPresets::DEFAULT_PRESET);
    }
    if ((!params.ip_address.empty ()) != (params.ip_port != 0))
    {
        safe_logger (spdlog::level::warn, "ip_address or ip_port is not specified");
    }
    // aux preset
    if ((!params.ip_address_aux.empty ()) && (params.ip_port_aux != 0))
    {
        MultiCastClient *client =
            new MultiCastClient (params.ip_address_aux.c_str (), params.ip_port_aux);
        clients.push_back (client);
        presets.push_back ((int)BrainFlowPresets::AUXILIARY_PRESET);
    }
    if ((!params.ip_address_aux.empty ()) != (params.ip_port_aux != 0))
    {
        safe_logger (spdlog::level::warn, "ip_address_aux or ip_port_aux is not specified");
    }
    // anc preset
    if ((!params.ip_address_anc.empty ()) && (params.ip_port_anc != 0))
    {
        MultiCastClient *client =
            new MultiCastClient (params.ip_address_anc.c_str (), params.ip_port_anc);
        clients.push_back (client);
        presets.push_back ((int)BrainFlowPresets::ANCILLARY_PRESET);
    }
    if ((!params.ip_address_anc.empty ()) != (params.ip_port_anc != 0))
    {
        safe_logger (spdlog::level::warn, "ip_address_anc or ip_port_anc is not specified");
    }

    if (clients.empty ())
    {
        safe_logger (spdlog::level::err, "No ip addresses and ports specified");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    int res = (int)BrainFlowExitCodes::STATUS_OK;
    initialized = true;
    for (auto client : clients)
    {
        int socket_res = client->init ();
        if (socket_res != (int)MultiCastReturnCodes::STATUS_OK)
        {
            log_socket_error (res);
            initialized = false;
            res = (int)BrainFlowExitCodes::GENERAL_ERROR;
            break;
        }
    }

    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        for (auto client : clients)
        {
            delete client;
        }
        clients.clear ();
        presets.clear ();
    }

    return res;
}

int StreamingBoard::config_board (std::string config, std::string &response)
{
    // dont allow streaming boards to change config for master board
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

int StreamingBoard::start_stream (int buffer_size, const char *streamer_params)
{
    if (keep_alive)
    {
        safe_logger (spdlog::level::err, "Streaming thread already running");
        return (int)BrainFlowExitCodes::STREAM_ALREADY_RUN_ERROR;
    }
    int res = prepare_for_acquisition (buffer_size, streamer_params);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }

    keep_alive = true;
    for (int i = 0; i < (int)clients.size (); i++)
    {
        streaming_threads.push_back (std::thread ([this, i] { this->read_thread (i); }));
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int StreamingBoard::stop_stream ()
{
    if (keep_alive)
    {
        keep_alive = false;
        for (std::thread &streaming_thread : streaming_threads)
        {
            streaming_thread.join ();
        }
        streaming_threads.clear ();
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    else
    {
        return (int)BrainFlowExitCodes::STREAM_THREAD_IS_NOT_RUNNING;
    }
}

int StreamingBoard::release_session ()
{
    if (initialized)
    {
        if (keep_alive)
        {
            stop_stream ();
        }
        free_packages ();
        initialized = false;
        for (auto client : clients)
        {
            delete client;
        }
        clients.clear ();
        presets.clear ();
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

void StreamingBoard::read_thread (int num)
{
    std::string preset_str = preset_to_string (presets[num]);
    if (board_descr.find (preset_str) == board_descr.end ())
    {
        safe_logger (spdlog::level::err, "invalid json or push_package args, no such key");
        return;
    }

    json board_preset = board_descr[preset_str];
    int num_rows = board_preset["num_rows"];
    int num_packages = get_brainflow_batch_size ();
    int transaction_len = num_rows * num_packages;
    int bytes_per_recv = sizeof (double) * transaction_len;
    double *transaction = new double[transaction_len];
    for (int i = 0; i < num_rows; i++)
    {
        transaction[i] = 0.0;
    }

    while (keep_alive)
    {
        int res = clients[num]->recv (transaction, bytes_per_recv);
        if (res != bytes_per_recv)
        {
            safe_logger (
                spdlog::level::trace, "unable to read {} bytes, read {}", bytes_per_recv, res);
            log_socket_error (-1);
            continue;
        }
        for (int i = 0; i < num_packages; i++)
        {
            push_package (transaction + i * num_rows, presets[num]);
        }
    }
    delete[] transaction;
}

void StreamingBoard::log_socket_error (int error_code)
{
#ifdef _WIN32
    safe_logger (spdlog::level::err, "WSAGetLastError is {}", WSAGetLastError ());
#else
    safe_logger (spdlog::level::err, "errno {} message {}", errno, strerror (errno));
#endif
    safe_logger (spdlog::level::err, "socket operation error code: {}", error_code);
}
