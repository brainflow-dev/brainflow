#pragma once

#include <stdint.h>
#include <string>
#include <thread>

#include "board.h"
#include "board_controller.h"

#include "broadcast_server.h"
#include "socket_client_udp.h"
#include "socket_server_tcp.h"

#define DEFAULT_CONTROL_PORT 12345
#define PACKET_DELIMITER_CSV '\n'


class BioListener : public Board
{

private:
    volatile bool keep_alive;
    volatile bool initialized;

    std::string ip_address;
    std::thread streaming_thread;
    SocketServerTCP *control_socket;
    std::mutex m;
    std::condition_variable cv;
    int control_port;
    int data_port;

    void read_thread ();
    std::vector<std::string> split_string (const std::string &package, char delim);

    int create_control_connection ();
    int send_control_msg (const char *msg);
    int wait_for_connection ();

public:
    BioListener (int board_id, struct BrainFlowInputParams params);
    ~BioListener ();

    int prepare_session ();
    int start_stream (int buffer_size, const char *streamer_params);
    int stop_stream ();
    int release_session ();
    int config_board (std::string config, std::string &response);
};
