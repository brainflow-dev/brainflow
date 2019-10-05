#pragma once

#include <thread>

#include "board.h"
#include "board_controller.h"
#include "socket_client.h"
#include "socket_server.h"


struct http_t; // forward declaration cause include "http.h" here leads to strange errors

class OpenBCIWifiShieldBoard : public Board
{

private:
    int wait_for_http_resp (http_t *request, int max_attempts = 3000);

protected:
    volatile bool keep_alive;
    bool initialized;
    std::thread streaming_thread;

    SocketServer *server_socket;
    int num_channels;
    char shield_ip[80];

    virtual void read_thread () = 0;

public:
    OpenBCIWifiShieldBoard (int num_channels, char *ip_addr);
    virtual ~OpenBCIWifiShieldBoard ();

    int prepare_session ();
    int start_stream (int buffer_size);
    int stop_stream ();
    int release_session ();
    int config_board (char *config);
};
