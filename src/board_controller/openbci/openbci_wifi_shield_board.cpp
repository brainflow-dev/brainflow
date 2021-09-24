#include <regex>
#include <sstream>
#include <string.h>
#include <string>

#include "openbci_wifi_shield_board.h"
#include "socket_client_udp.h"

#define HTTP_IMPLEMENTATION
#include "http.h"


constexpr int OpenBCIWifiShieldBoard::package_size;

OpenBCIWifiShieldBoard::OpenBCIWifiShieldBoard (struct BrainFlowInputParams params, int board_id)
    : Board (board_id, params)
{
    server_socket = NULL;
    keep_alive = false;
    initialized = false;
    http_timeout = 10;
}

OpenBCIWifiShieldBoard::~OpenBCIWifiShieldBoard ()
{
    skip_logs = true;
    release_session ();
}

int OpenBCIWifiShieldBoard::prepare_session ()
{
    if (initialized)
    {
        safe_logger (spdlog::level::info, "Session already prepared");
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    if (params.ip_address.empty ())
    {
        params.ip_address = find_wifi_shield ();
    }
    if ((params.timeout > 0) && (params.timeout < 600))
    {
        http_timeout = params.timeout;
    }
    safe_logger (spdlog::level::info, "use {} as http timeout", http_timeout);
    // user doent need to provide this param because we have only tcp impl,
    // but if its specified and its UDP return an error
    if (params.ip_protocol == (int)IpProtocolType::UDP)
    {
        safe_logger (spdlog::level::err, "ip protocol should be tcp");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    if (!params.ip_port)
    {
        safe_logger (spdlog::level::err, "ip port is empty");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    char local_ip[80];
    int res = SocketClientUDP::get_local_ip_addr (params.ip_address.c_str (), 80, local_ip);
    if (res != 0)
    {
        safe_logger (spdlog::level::err, "failed to get local ip addr: {}", res);
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    safe_logger (spdlog::level::info, "local ip addr is {}", local_ip);

    server_socket = new SocketServerTCP (local_ip, params.ip_port, true);
    // bind socket
    res = server_socket->bind ();
    if (res != (int)SocketServerTCPReturnCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to create server socket with addr {} and port {}",
            local_ip, params.ip_port);
        delete server_socket;
        server_socket = NULL;
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    safe_logger (spdlog::level::trace, "bind socket, port  is {}", params.ip_port);

    // run accept in another thread to dont block
    res = server_socket->accept ();
    if (res != (int)SocketServerTCPReturnCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "error in accept");
        delete server_socket;
        server_socket = NULL;
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }

    std::string url = "http://" + params.ip_address + "/board";
    http_t *request = http_get (url.c_str (), NULL);
    if (!request)
    {
        safe_logger (spdlog::level::err, "error during request creation, to {}", url.c_str ());
        delete server_socket;
        server_socket = NULL;
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    res = wait_for_http_resp (request);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        http_release (request);
        delete server_socket;
        server_socket = NULL;
        return res;
    }
    http_release (request);

    // send info about tcp socket we created
    url = "http://" + params.ip_address + "/tcp";
    json post_data;
    post_data["ip"] = std::string (local_ip);
    post_data["port"] = params.ip_port;
    post_data["output"] = std::string ("raw");
    post_data["delimiter"] = true;
    post_data["latency"] = 10000;
    std::string post_str = post_data.dump ();
    safe_logger (spdlog::level::info, "configuration string {}", post_str.c_str ());
    request = http_post (url.c_str (), post_str.c_str (), strlen (post_str.c_str ()), NULL);
    if (!request)
    {
        safe_logger (spdlog::level::err, "error during request creation, to {}", url.c_str ());
        delete server_socket;
        server_socket = NULL;
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    int send_res = wait_for_http_resp (request);
    if (send_res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        http_release (request);
        delete server_socket;
        server_socket = NULL;
        return send_res;
    }
    http_release (request);

    // waiting for accept call
    int max_attempts = 10;
    for (int i = 0; i < max_attempts; i++)
    {
        safe_logger (spdlog::level::trace, "waiting for accept {}/{}", i, max_attempts);
        if (server_socket->client_connected)
        {
            safe_logger (spdlog::level::trace, "connected");
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
    if (!server_socket->client_connected)
    {
        safe_logger (spdlog::level::trace, "failed to establish connection");
        delete server_socket;
        server_socket = NULL;
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }

    // freeze sampling rate
    initialized = true;
    res = send_config ("~4"); // for cyton based boards - 1000 for ganglion - 1600
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        initialized = false;
        delete server_socket;
        server_socket = NULL;
        return res;
    }

    return (int)BrainFlowExitCodes::STATUS_OK;
}


int OpenBCIWifiShieldBoard::send_config (const char *config)
{
    if (!initialized)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }

    std::string url = "http://" + params.ip_address + "/command";
    json post_data;
    post_data["command"] = std::string (config);
    std::string post_str = post_data.dump ();
    safe_logger (spdlog::level::info, "command string {}", post_str.c_str ());
    http_t *request = http_post (url.c_str (), post_str.c_str (), strlen (post_str.c_str ()), NULL);
    if (!request)
    {
        safe_logger (spdlog::level::err, "error during request creation, to {}", url.c_str ());
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    int send_res = wait_for_http_resp (request);
    if (send_res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        http_release (request);
        return send_res;
    }
    http_release (request);

    if (keep_alive)
    {
        safe_logger (spdlog::level::warn,
            "You are changing board params during streaming, it may lead to sync mismatch between "
            "data acquisition thread and device");
    }
    safe_logger (spdlog::level::warn, "If you change gain you may need to rescale EXG data");

    return (int)BrainFlowExitCodes::STATUS_OK;
}

int OpenBCIWifiShieldBoard::config_board (std::string config, std::string &response)
{
    return send_config (config.c_str ());
}

int OpenBCIWifiShieldBoard::start_stream (int buffer_size, const char *streamer_params)
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

    std::string url = "http://" + params.ip_address + "/stream/start";
    http_t *request = http_get (url.c_str (), NULL);
    if (!request)
    {
        safe_logger (spdlog::level::err, "error during request creation, to {}", url.c_str ());
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    int send_res = wait_for_http_resp (request);
    if (send_res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        http_release (request);
        return send_res;
    }
    http_release (request);

    keep_alive = true;
    streaming_thread = std::thread ([this] { this->read_thread (); });
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int OpenBCIWifiShieldBoard::stop_stream ()
{
    if (keep_alive)
    {
        keep_alive = false;
        streaming_thread.join ();
        std::string url = "http://" + params.ip_address + "/stream/stop";
        http_t *request = http_get (url.c_str (), NULL);
        if (!request)
        {
            safe_logger (spdlog::level::err, "error during request creation, to {}", url.c_str ());
            return (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
        int send_res = wait_for_http_resp (request);
        if (send_res != (int)BrainFlowExitCodes::STATUS_OK)
        {
            http_release (request);
            return send_res;
        }
        http_release (request);
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    else
    {
        return (int)BrainFlowExitCodes::STREAM_THREAD_IS_NOT_RUNNING;
    }
}

int OpenBCIWifiShieldBoard::release_session ()
{
    if (initialized)
    {
        if (keep_alive)
        {
            stop_stream ();
        }
        free_packages ();
        initialized = false;
    }
    if (server_socket)
    {
        delete server_socket;
        server_socket = NULL;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

std::string OpenBCIWifiShieldBoard::find_wifi_shield ()
{
    safe_logger (spdlog::level::info, "trying to autodiscover wifi shield using SSDP");
    // try to find device using SSDP if there is an error try to use ip address for direct mode
    // instead throwing exception
    std::string ip_address = "192.168.4.1";

    SocketClientUDP udp_client ("239.255.255.250", 1900); // ssdp ip and port
    int res = udp_client.connect ();
    if (res == (int)SocketClientUDPReturnCodes::STATUS_OK)
    {
        std::string msearch =
            ("M-SEARCH * HTTP/1.1\r\nHost: 239.255.255.250:1900\r\nMAN: ssdp:discover\r\n"
             "ST: urn:schemas-upnp-org:device:Basic:1\r\n"
             "MX: 3\r\n"
             "\r\n"
             "\r\n");

        safe_logger (spdlog::level::trace, "Using search request {}", msearch.c_str ());

        res = (int)udp_client.send (msearch.c_str (), (int)msearch.size ());
        if (res == msearch.size ())
        {
            unsigned char b[250];
            res = udp_client.recv (b, 250);
            if (res == 250)
            {
                std::string response ((const char *)b);
                safe_logger (spdlog::level::trace, "Recived package {}", b);
                std::regex rgx ("LOCATION: http://([0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+)");
                std::smatch matches;
                if (std::regex_search (response, matches, rgx) == true)
                {
                    if (matches.size () == 2)
                    {
                        ip_address = matches.str (1);
                    }
                    else
                    {
                        safe_logger (spdlog::level::err, "invalid number of groups found");
                    }
                }
                else
                {
                    safe_logger (spdlog::level::err, "failed to find shield ip address");
                }
            }
            else
            {
                safe_logger (spdlog::level::err, "Recv res {}", res);
            }
        }
        else
        {
            safe_logger (spdlog::level::err, "Sent res {}", res);
        }
    }
    else
    {
        safe_logger (spdlog::level::err, "Failed to connect socket {}", res);
    }

    udp_client.close ();

    safe_logger (spdlog::level::info, "use ip address {}", ip_address.c_str ());
    return ip_address;
}

int OpenBCIWifiShieldBoard::wait_for_http_resp (http_t *request)
{
    http_status_t status = HTTP_STATUS_PENDING;
    int prev_size = -1;
    int sleep_interval = 10;
    int max_attempts = (http_timeout * 1000) / sleep_interval;
    int i = 0;
    while (status == HTTP_STATUS_PENDING)
    {
        i++;
        if (i == max_attempts)
        {
            safe_logger (spdlog::level::err, "still pending after {} seconds", http_timeout);
            return (int)BrainFlowExitCodes::BOARD_WRITE_ERROR;
        }
        status = http_process (request);
        if (prev_size != (int)request->response_size)
        {
            safe_logger (spdlog::level::trace, "recieved {} bytes", (int)request->response_size);
            prev_size = (int)request->response_size;
        }
#ifdef _WIN32
        Sleep ((int)(sleep_interval));
#else
        usleep ((int)(sleep_interval * 1000));
#endif
    }
    if (request->response_data != NULL)
    {
        safe_logger (
            spdlog::level::trace, "response data {}", (char const *)request->response_data);
    }
    if (status == HTTP_STATUS_FAILED)
    {
        safe_logger (spdlog::level::err, "request failed");
        return (int)BrainFlowExitCodes::BOARD_WRITE_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}
