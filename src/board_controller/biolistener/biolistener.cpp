#include "biolistener.h"

#include <chrono>
#include <sstream>
#include <stdint.h>
#include <string.h>

#include "custom_cast.h"
#include "json.hpp"
#include "network_interfaces.h"
#include "timestamp.h"

using json = nlohmann::json;


BioListener::BioListener(int board_id, struct BrainFlowInputParams params) : Board (board_id, params)
{
    control_socket = NULL;
    keep_alive = false;
    initialized = false;
    control_port = -1;
    data_port = -1;
}

BioListener::~BioListener ()
{
    skip_logs = true;
    release_session ();
}

int BioListener::prepare_session ()
{
    if (initialized)
    {
        safe_logger (spdlog::level::info, "Session is already prepared");
        return (int)BrainFlowExitCodes::STATUS_OK;
    }

    if (params.timeout < 2)
    {
        params.timeout = 4;
    }

    int res = create_control_connection ();

    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        res = wait_for_connection ();
    }
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        res = send_control_msg (R"({"command":"send_control_msg"})");
    }

    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        if (control_socket != NULL)
        {
            delete control_socket;
            control_socket = NULL;
        }
    }
    else
    {
        initialized = true;
    }

    return res;
}

int BioListener::config_board (std::string conf, std::string &response)
{
    return send_control_msg (conf.c_str ());
}

int BioListener::start_stream (int buffer_size, const char *streamer_params)
{
    if (!initialized)
    {
        safe_logger (spdlog::level::err, "You need to call prepare_session before config_board");
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
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
    res = send_control_msg (R"({"command":"start_stream"})");
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        keep_alive = true;
        streaming_thread = std::thread ([this] { this->read_thread (); });
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int BioListener::stop_stream ()
{
    if (keep_alive)
    {
        if (send_control_msg (R"({"command":"stop_stream")") != (int)BrainFlowExitCodes::STATUS_OK)
        {
            safe_logger (spdlog::level::warn, "failed to set low power mode");
        }
        keep_alive = false;
        streaming_thread.join ();
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    return (int)BrainFlowExitCodes::STREAM_THREAD_IS_NOT_RUNNING;
}

int BioListener::release_session ()
{
    if (initialized)
    {
        if (keep_alive)
        {
            stop_stream ();
        }
        initialized = false;
        free_packages ();
        send_control_msg (R"({"command":"release_session"})");
        if (control_socket)
        {
            control_socket->close ();
            delete control_socket;
            control_socket = NULL;
        }
        control_port = -1;
        data_port = -1;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

void BioListener::read_thread ()
{
    const int max_size = 1024;
    char message[max_size];
    int num_rows = board_descr["default"]["num_rows"];
    double *package = new double[num_rows];
    for (int i = 0; i < num_rows; i++)
    {
        package[i] = 0.0;
    }

    std::vector<int> eeg_channels = board_descr["default"]["eeg_channels"];
    std::vector<int> other_channels = board_descr["default"]["other_channels"];

    while (keep_alive)
    {
        int bytes_recv = control_socket->recv (message, max_size);
        if (bytes_recv < 1)
        {
            safe_logger (spdlog::level::trace, "no data received");
            continue;
        }
        std::string message_received = std::string (message, bytes_recv);
        std::vector<std::string> splitted_packages =
            split_string (message_received, PACKET_DELIMITER_CSV);

        safe_logger (spdlog::level::trace, "Received message: >>>{}<<<", message_received.c_str ());


        for (std::string recv_package : splitted_packages)
        {
            safe_logger (spdlog::level::trace, "Received package: >>>{}<<<", recv_package.c_str ());
            try {
                json j = json::parse (recv_package);
//                json j = json::from_msgpack(recv_package);

                safe_logger (spdlog::level::trace, "Parsed json: {}", j.dump ());


                // parse from_msgpack


                if (j["type"] == 1)
                {
                    package[board_descr["default"]["timestamp_channel"].get<int> ()] = j["ts"];
                    package[board_descr["default"]["package_num_channel"].get<int> ()] = j["n"];
//                    int sensor_id = j["s_id"];

                    // in data is array of 8 values, copy them to package
                    for (int i = 0; i < 8; i++)
                    {
                        package[eeg_channels[i]] = j["data"][i];
                    }


                    push_package (package);
                }


            } catch (json::parse_error &e) {
                safe_logger (spdlog::level::err, "Failed to parse json: {}", e.what ());
            }


        }
    }

    delete[] package;
}

std::vector<std::string> BioListener::split_string (const std::string &package, char delim)
{
    std::vector<std::string> result;
    size_t start;
    size_t end = 0;
    while ((start = package.find_first_not_of (delim, end)) != std::string::npos)
    {
        end = package.find (delim, start);
        std::string cur_str = package.substr (start, end - start);
        result.push_back (cur_str);
    }
    return result;
}


int BioListener::create_control_connection ()
{
    char local_ip[80];

    strncpy(local_ip, params.ip_address.c_str(), sizeof(local_ip) - 1);
    local_ip[sizeof(local_ip) - 1] = '\0';
//    safe_logger (spdlog::level::info, "Local ip address is {}", params.ip_address.c_str ());
//    int local_ip_res =
//        SocketClientUDP::get_local_ip_addr (params.ip_address.c_str (), DEFAULT_CONTROL_PORT, local_ip);
//    if (local_ip_res != (int)SocketClientUDPReturnCodes::STATUS_OK)
//    {
//        safe_logger (spdlog::level::err, "failed to get local ip addr: {}", local_ip_res);
//        return (int)BrainFlowExitCodes::GENERAL_ERROR;
//    }
    safe_logger (spdlog::level::info, "local ip addr is {}", local_ip);

    int res = (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    for (int i = 0; i < 39; i += 2)
    {
        control_port = DEFAULT_CONTROL_PORT + i;
        control_socket = new SocketServerTCP (local_ip, control_port, false);
        if (control_socket->bind () == ((int)SocketServerTCPReturnCodes::STATUS_OK))
        {
            safe_logger (spdlog::level::info, "use port {} for control", control_port);
            res = (int)BrainFlowExitCodes::STATUS_OK;
            break;
        }
        else
        {
            safe_logger (spdlog::level::warn, "failed to connect to {}", control_port);
        }
        control_socket->close ();
        delete control_socket;
        control_socket = NULL;
    }
    return res;
}

int BioListener::send_control_msg (const char *msg)
{
    // FIXME: hardcoded ports
//    // should never happen
//    if ((control_port < 0) || (data_port < 0))
//    {
//        safe_logger (spdlog::level::info, "ports for data or control are not set");
//        return (int)BrainFlowExitCodes::GENERAL_ERROR;
//    }

    // convert msg to string just by copying
    std::string package = msg;

    int res = (int)BrainFlowExitCodes::STATUS_OK;
    int bytes_send = control_socket->send (package.c_str (), (int)package.size ());
    if (bytes_send != (int)package.size ())
    {
        safe_logger (spdlog::level::err, "failed to send control msg package: {}, res is {}", msg,
            bytes_send);
        res = (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    else
    {
        safe_logger (spdlog::level::info, "Message: {} sent", msg);
    }
    return res;
}

int BioListener::wait_for_connection ()
{
    int res = (int)BrainFlowExitCodes::STATUS_OK;
    int accept_res = control_socket->accept ();
    if (accept_res != (int)SocketServerTCPReturnCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "error in accept");
        res = (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    else
    {
        int max_attempts = 1500;
        for (int i = 0; i < max_attempts; i++)
        {
            safe_logger (spdlog::level::trace, "waiting for accept {}/{}", i, max_attempts);
            if (control_socket->client_connected)
            {
                safe_logger (spdlog::level::trace, "emotibit connected");
                break;
            }
            else
            {
#ifdef _WIN32
                Sleep (300);
#else
                usleep (300000);
#endif
            }
        }
        if (!control_socket->client_connected)
        {
            safe_logger (spdlog::level::trace, "failed to establish connection");
            res = (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
        }
    }
    return res;
}


