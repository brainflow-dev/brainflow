#include <sstream>
#include <string.h>
#include <string>

#include "openbci_helpers.h"
#include "openbci_wifi_shield_board.h"

#include "json.hpp"

#define HTTP_IMPLEMENTATION
#include "http.h"

using json = nlohmann::json;

constexpr int OpenBCIWifiShieldBoard::package_size;

OpenBCIWifiShieldBoard::OpenBCIWifiShieldBoard (
    int num_channels, struct BrainFlowInputParams params, int board_id)
    : Board (board_id, params)
{
    this->num_channels = num_channels;
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
        return STATUS_OK;
    }
    if (params.ip_address.empty ())
    {
        safe_logger (spdlog::level::warn, "use default ip address 192.168.4.1");
        params.ip_address = "192.168.4.1";
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
        return INVALID_ARGUMENTS_ERROR;
    }
    if (!params.ip_port)
    {
        safe_logger (spdlog::level::err, "ip port is empty");
        return INVALID_ARGUMENTS_ERROR;
    }
    char local_ip[80];
    int res = SocketClient::get_local_ip_addr (
        const_cast<char *> (params.ip_address.c_str ()), 80, local_ip);
    if (res != 0)
    {
        safe_logger (spdlog::level::err, "failed to get local ip addr: {}", res);
        return GENERAL_ERROR;
    }
    safe_logger (spdlog::level::info, "local ip addr is {}", local_ip);

    server_socket = new SocketServer (local_ip, params.ip_port);
    res = server_socket->bind (
        OpenBCIWifiShieldBoard::package_size); // set min bytes returned by recv
    if (res != 0)
    {
        safe_logger (spdlog::level::err, "failed to create server socket with addr {} and port {}",
            local_ip, params.ip_port);
        return GENERAL_ERROR;
    }
    safe_logger (spdlog::level::trace, "bind socket, port  is {}", params.ip_port);

    std::string url = "http://" + params.ip_address + "/board";
    http_t *request = http_get (url.c_str (), NULL);
    if (!request)
    {
        safe_logger (spdlog::level::err, "error during request creation, to {}", url.c_str ());
        return GENERAL_ERROR;
    }
    res = wait_for_http_resp (request);
    if (res != STATUS_OK)
    {
        http_release (request);
        return res;
    }
    http_release (request);

    res = server_socket->accept ();
    if (res != 0)
    {
        safe_logger (spdlog::level::err, "error in accept");
        return GENERAL_ERROR;
    }
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
        return GENERAL_ERROR;
    }
    int send_res = wait_for_http_resp (request);
    if (send_res != STATUS_OK)
    {
        http_release (request);
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
        server_socket->close ();
        delete server_socket;
        server_socket = NULL;
        return BOARD_NOT_READY_ERROR;
    }

    // freeze sampling rate
    initialized = true;
    res = config_board ("~4"); // for cyton based boards - 1000 for ganglion - 1600
    if (res != STATUS_OK)
    {
        initialized = false;
        return res;
    }

    return STATUS_OK;
}


int OpenBCIWifiShieldBoard::send_config (char *config)
{
    if (!initialized)
    {
        return BOARD_NOT_READY_ERROR;
    }
    int res = validate_config (config);
    if (res != STATUS_OK)
    {
        return res;
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
        return GENERAL_ERROR;
    }
    int send_res = wait_for_http_resp (request);
    if (send_res != STATUS_OK)
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

    return STATUS_OK;
}

int OpenBCIWifiShieldBoard::config_board (char *config)
{
    return send_config (config);
}

int OpenBCIWifiShieldBoard::start_stream (int buffer_size, char *streamer_params)
{
    if (keep_alive)
    {
        safe_logger (spdlog::level::err, "Streaming thread already running");
        return STREAM_ALREADY_RUN_ERROR;
    }
    if (buffer_size <= 0 || buffer_size > MAX_CAPTURE_SAMPLES)
    {
        safe_logger (spdlog::level::err, "invalid array size");
        return INVALID_BUFFER_SIZE_ERROR;
    }

    if (db)
    {
        delete db;
        db = NULL;
    }
    if (streamer)
    {
        delete streamer;
        streamer = NULL;
    }

    int res = prepare_streamer (streamer_params);
    if (res != STATUS_OK)
    {
        return res;
    }
    db = new DataBuffer (num_channels, buffer_size);
    if (!db->is_ready ())
    {
        safe_logger (spdlog::level::err, "unable to prepare buffer");
        delete db;
        db = NULL;
        return INVALID_BUFFER_SIZE_ERROR;
    }

    std::string url = "http://" + params.ip_address + "/stream/start";
    http_t *request = http_get (url.c_str (), NULL);
    if (!request)
    {
        safe_logger (spdlog::level::err, "error during request creation, to {}", url.c_str ());
        return GENERAL_ERROR;
    }
    int send_res = wait_for_http_resp (request);
    if (send_res != STATUS_OK)
    {
        http_release (request);
        return send_res;
    }
    http_release (request);

    keep_alive = true;
    streaming_thread = std::thread ([this] { this->read_thread (); });
    return STATUS_OK;
}

int OpenBCIWifiShieldBoard::stop_stream ()
{
    if (keep_alive)
    {
        keep_alive = false;
        streaming_thread.join ();
        if (streamer)
        {
            delete streamer;
            streamer = NULL;
        }
        std::string url = "http://" + params.ip_address + "/stream/stop";
        http_t *request = http_get (url.c_str (), NULL);
        if (!request)
        {
            safe_logger (spdlog::level::err, "error during request creation, to {}", url.c_str ());
            return GENERAL_ERROR;
        }
        int send_res = wait_for_http_resp (request);
        if (send_res != STATUS_OK)
        {
            http_release (request);
            return send_res;
        }
        http_release (request);
        return STATUS_OK;
    }
    else
    {
        return STREAM_THREAD_IS_NOT_RUNNING;
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
        initialized = false;
    }
    if (server_socket)
    {
        server_socket->close ();
        delete server_socket;
        server_socket = NULL;
    }
    return STATUS_OK;
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
            return BOARD_WRITE_ERROR;
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
        return BOARD_WRITE_ERROR;
    }
    return STATUS_OK;
}
