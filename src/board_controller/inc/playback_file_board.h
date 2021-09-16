#pragma once

#include <condition_variable>
#include <mutex>
#include <thread>

#include "board.h"
#include "board_controller.h"


class PlaybackFileBoard : public Board
{

private:
    volatile bool keep_alive;
    volatile bool loopback;
    volatile bool use_new_timestamps;
    std::thread streaming_thread;
    bool is_streaming;
    bool initialized;
    std::mutex m;
    std::condition_variable cv;
    volatile int state;

    void read_thread ();

public:
    PlaybackFileBoard (struct BrainFlowInputParams params);
    ~PlaybackFileBoard ();

    int prepare_session ();
    int start_stream (int buffer_size, const char *streamer_params);
    int stop_stream ();
    int release_session ();
    int config_board (std::string config, std::string &response);
};
