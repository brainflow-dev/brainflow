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
    NotionOSC (int board_id, struct BrainFlowInputParams params);
    ~NotionOSC () override;

    int prepare_session () override;
    int start_stream (int buffer_size, const char *streamer_params) override;
    int stop_stream () override;
    int release_session () override;
    int config_board (std::string config, std::string &response) override;
};
