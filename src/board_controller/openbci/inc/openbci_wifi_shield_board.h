#pragma once

#include <string>
#include <thread>

#include "board.h"
#include "board_controller.h"
#include "socket_server_tcp.h"


struct http_t; // forward declaration cause include "http.h" here leads to strange errors

class OpenBCIWifiShieldBoard : public Board
{

protected:
    int http_timeout;
    int wait_for_http_resp (http_t *request);

    volatile bool keep_alive;
    bool initialized;
    std::thread streaming_thread;

    SocketServerTCP *server_socket;

    std::string find_wifi_shield ();

    virtual void read_thread () = 0;
    virtual int send_config (const char *config);

public:
    OpenBCIWifiShieldBoard (struct BrainFlowInputParams params, int board_id);
    virtual ~OpenBCIWifiShieldBoard ();

    virtual int prepare_session ();
    virtual int start_stream (int buffer_size, const char *streamer_params);
    virtual int stop_stream ();
    virtual int release_session ();
    virtual int config_board (std::string config, std::string &response);

    static constexpr int package_size = 33;
};
