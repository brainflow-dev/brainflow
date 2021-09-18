#pragma once

#include <thread>

#include "board.h"
#include "board_controller.h"
#include "multicast_client.h"


class StreamingBoard : public Board
{

private:
    volatile bool keep_alive;
    bool initialized;
    bool is_streaming;
    std::thread streaming_thread;

    MultiCastClient *client;

    void read_thread ();

public:
    StreamingBoard (struct BrainFlowInputParams params);
    ~StreamingBoard ();

    int prepare_session ();
    int start_stream (int buffer_size, const char *streamer_params);
    int stop_stream ();
    int release_session ();
    int config_board (std::string config, std::string &response);
};
