#pragma once

#include <condition_variable>
#include <math.h>
#include <mutex>
#include <thread>

#include "board.h"
#include "board_controller.h"
#include "broadcast_client.h"

#include <oscpp/client.hpp>
#include <oscpp/server.hpp>


class NotionOSC : public Board
{

private:
    volatile bool keep_alive;
    bool initialized;
    std::thread streaming_thread;
    BroadCastClient *socket;

    std::mutex m;
    std::condition_variable cv;
    volatile int state;
    void read_thread ();

    void handle_packet (double *package, const OSCPP::Server::Packet &packet);

public:
    NotionOSC (struct BrainFlowInputParams params);
    ~NotionOSC ();

    int prepare_session ();
    int start_stream (int buffer_size, const char *streamer_params);
    int stop_stream ();
    int release_session ();
    int config_board (std::string config, std::string &response);
};
