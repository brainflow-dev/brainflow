#include "emotibit.h"

#include <chrono>
#include <sstream>
#include <stdint.h>
#include <string.h>

#include "custom_cast.h"
#include "json.hpp"
#include "timestamp.h"

#include "emotibit_defines.h"

using json = nlohmann::json;


Emotibit::Emotibit (struct BrainFlowInputParams params)
    : Board ((int)BoardIds::EMOTIBIT_BOARD, params)
{
    data_socket = NULL;
    control_socket = NULL;
    advertise_socket_server = NULL;
    keep_alive = false;
    initialized = false;
    state = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
}

Emotibit::~Emotibit ()
{
    skip_logs = true;
    release_session ();
}

int Emotibit::prepare_session ()
{
    if (initialized)
    {
        safe_logger (spdlog::level::info, "Session is already prepared");
        return (int)BrainFlowExitCodes::STATUS_OK;
    }

    if (params.timeout < 2)
    {
        params.timeout = 6;
    }

    int res = create_adv_connection ();
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        res = create_data_connection ();
    }
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        res = create_control_connection ();
    }

    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        if (advertise_socket_server != NULL)
        {
            delete advertise_socket_server;
            advertise_socket_server = NULL;
        }
        if (data_socket != NULL)
        {
            delete data_socket;
            data_socket = NULL;
        }
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

int Emotibit::config_board (std::string conf, std::string &response)
{
    safe_logger (spdlog::level::err, "config board is not supported for emotibit");
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

int Emotibit::start_stream (int buffer_size, const char *streamer_params)
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
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int Emotibit::stop_stream ()
{
    if (keep_alive)
    {
        keep_alive = false;
        streaming_thread.join ();
        this->state = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    return (int)BrainFlowExitCodes::STREAM_THREAD_IS_NOT_RUNNING;
}

int Emotibit::release_session ()
{
    if (initialized)
    {
        if (keep_alive)
        {
            stop_stream ();
        }
        free_packages ();
        if (data_socket)
        {
            data_socket->close ();
            delete data_socket;
            data_socket = NULL;
        }
        if (control_socket)
        {
            control_socket->close ();
            delete control_socket;
            control_socket = NULL;
        }
        if (advertise_socket_server)
        {
            advertise_socket_server->close ();
            delete advertise_socket_server;
            advertise_socket_server = NULL;
        }
        initialized = false;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

void Emotibit::read_thread ()
{
    while (keep_alive)
    {
    }
}

std::string Emotibit::create_package (const std::string &type_tag, uint16_t packet_number,
    const std::string &data, uint16_t data_length, uint8_t protocol_version,
    uint8_t data_reliability)
{
    std::string header =
        create_header (type_tag, 0, packet_number, data_length, protocol_version, data_reliability);
    if (data_length == 0)
    {
        return header + PACKET_DELIMITER_CSV;
    }
    else
    {
        return header + PAYLOAD_DELIMITER + data + PACKET_DELIMITER_CSV;
    }
}

std::string Emotibit::create_package (const std::string &type_tag, uint16_t packet_number,
    const std::vector<std::string> &data, uint8_t protocol_version, uint8_t data_reliability)
{
    std::string package = create_header (
        type_tag, 0, packet_number, (uint16_t)data.size (), protocol_version, data_reliability);
    for (std::string s : data)
    {
        package += PAYLOAD_DELIMITER + s;
    }
    package += PACKET_DELIMITER_CSV;
    return package;
}

std::string Emotibit::create_header (const std::string &type_tag, uint32_t timestamp,
    uint16_t packet_number, uint16_t data_length, uint8_t protocol_version,
    uint8_t data_reliability)
{
    std::string header = "";
    header += std::to_string (timestamp);
    header += PAYLOAD_DELIMITER;
    header += std::to_string (packet_number);
    header += PAYLOAD_DELIMITER;
    header += std::to_string (data_length);
    header += PAYLOAD_DELIMITER;
    header += type_tag;
    header += PAYLOAD_DELIMITER;
    header += std::to_string ((int)protocol_version);
    header += PAYLOAD_DELIMITER;
    header += std::to_string ((int)data_reliability);
    return header;
}

std::vector<std::string> Emotibit::split_string (const std::string &package, char delim)
{
    std::vector<std::string> result;
    std::stringstream ss (package);
    std::string item;

    while (getline (ss, item, delim))
    {
        result.push_back (item);
    }

    return result;
}

bool Emotibit::get_header (
    const std::string &package_string, int *package_num, int *data_len, std::string &type_tag)
{
    std::vector<std::string> package = split_string (package_string, PAYLOAD_DELIMITER);
    for (std::string s : package)
    {
        safe_logger (spdlog::level::trace, "{}", s.c_str ());
    }
    if (package.size () >= HEADER_LENGTH)
    {
        try
        {
            if (package.at (1) != "")
            {
                *package_num = stoi (package.at (1));
            }
            else
            {
                return false;
            }
            if (package.at (2) != "")
            {
                *data_len = stoi (package.at (2));
            }
            else
            {
                return false;
            }
            if (package.at (3) != "")
            {
                type_tag = package.at (3);
            }
            else
            {
                return false;
            }
        }
        catch (...)
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    if (package.size () < (size_t)HEADER_LENGTH + *data_len)
    {
        return false;
    }
    else
    {
        return true;
    }
}

int Emotibit::create_adv_connection ()
{
    int res = (int)BrainFlowExitCodes::STATUS_OK;
    if (params.ip_address.empty ())
    {
        // todo implement search for available networks and make this param optional,
        // 255.255.255.255 doesnt work(no response from emotibit)
        safe_logger (spdlog::level::err, "no ip address for broadcast advertising provided.");
        res = (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        advertise_socket_server =
            new BroadCastServer (params.ip_address.c_str (), WIFI_ADVERTISING_PORT);
        int init_res = advertise_socket_server->init ();
        if (init_res != (int)BroadCastServerReturnCodes::STATUS_OK)
        {
            safe_logger (spdlog::level::err, "failed to init broadcast server socket: {}", res);
            res = (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
    }
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        std::string package = create_package (HELLO_EMOTIBIT, 0, "", 0);
        safe_logger (spdlog::level::info, "sending package: {}", package.c_str ());
        int bytes_send = advertise_socket_server->send (package.c_str (), (int)package.size ());
        if (bytes_send != (int)package.size ())
        {
            safe_logger (spdlog::level::err, "failed to send adv package, res is {}", bytes_send);
            res = (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
    }
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        constexpr int max_size = 32768;
        constexpr int max_ip_addr_size = 100;
        char recv_data[max_size];
        char emotibit_ip[max_ip_addr_size];
        bool found = false;
        double start_time = get_timestamp ();
        for (int i = 0; (i < 100) && (!found); i++)
        {
            int bytes_recv =
                advertise_socket_server->recv (recv_data, max_size, emotibit_ip, max_ip_addr_size);
            if (bytes_recv > 0)
            {
                std::vector<std::string> splitted_packages =
                    split_string (std::string (recv_data, bytes_recv), PACKET_DELIMITER_CSV);
                for (std::string recv_package : splitted_packages)
                {
                    safe_logger (spdlog::level::trace, "package is {}", recv_package.c_str ());
                    int package_num = 0;
                    int data_len = 0;
                    std::string type_tag = "";
                    if (get_header (recv_package, &package_num, &data_len, type_tag))
                    {
                        safe_logger (spdlog::level::info, "received {} package", type_tag);
                        if ((type_tag == HELLO_HOST) || (type_tag == PONG))
                        {
                            found = true;
                            ip_address = emotibit_ip;
                        }
                    }
                    else
                    {
                        for (int j = 0; j < bytes_recv; j++)
                        {
                            safe_logger (
                                spdlog::level::trace, "byte {} {}", j, (unsigned char)recv_data[j]);
                        }
                    }
                }
            }
            if (get_timestamp () - start_time > params.timeout)
            {
                break;
            }
        }
        if (!found)
        {
            safe_logger (spdlog::level::err, "no emotibit found");
            res = (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
        }
    }
    return res;
}

int Emotibit::create_data_connection ()
{
    int res = (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    for (int i = 2; i < 40; i += 2)
    {
        int data_port = WIFI_ADVERTISING_PORT + i;
        data_socket = new SocketClientUDP (ip_address.c_str (), data_port);
        if (data_socket->connect () == ((int)SocketClientUDPReturnCodes::STATUS_OK))
        {
            if (data_socket->bind () == ((int)SocketClientUDPReturnCodes::STATUS_OK))
            {
                safe_logger (spdlog::level::info, "use port {} for data", data_port);
                res = (int)BrainFlowExitCodes::STATUS_OK;
                break;
            }
            else
            {
                safe_logger (spdlog::level::warn, "failed to bind to {}", data_port);
            }
        }
        else
        {
            safe_logger (spdlog::level::warn, "failed to connect to {}", data_port);
        }
        data_socket->close ();
        delete data_socket;
        data_socket = NULL;
    }
    return res;
}

int Emotibit::create_control_connection ()
{
    char local_ip[80];
    int local_ip_res =
        SocketClientUDP::get_local_ip_addr (ip_address.c_str (), WIFI_ADVERTISING_PORT, local_ip);
    if (local_ip_res != (int)SocketClientUDPReturnCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to get local ip addr: {}", local_ip_res);
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    safe_logger (spdlog::level::info, "local ip addr is {}", local_ip);

    int res = (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    for (int i = 1; i < 39; i += 2)
    {
        int control_port = WIFI_ADVERTISING_PORT + i;
        control_socket = new SocketServerTCP (local_ip, control_port, true);
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