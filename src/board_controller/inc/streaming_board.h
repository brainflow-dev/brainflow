#pragma once

#include <string>
#include <thread>
#include <vector>

#include "board.h"
#include "board_controller.h"
#include "multicast_client.h"


class StreamingBoard : public Board
{

private:
    volatile bool keep_alive;
    bool initialized;
    std::vector<std::thread> streaming_threads;
    std::vector<MultiCastClient *> clients;
    std::vector<int> presets;

    void read_thread (int num);
    void log_socket_error (int error_code);

public:
    StreamingBoard (struct BrainFlowInputParams params);
    ~StreamingBoard ();

    int prepare_session ();
    int start_stream (int buffer_size, const char *streamer_params);
    int stop_stream ();
    int release_session ();
    int config_board (std::string config, std::string &response);
};
