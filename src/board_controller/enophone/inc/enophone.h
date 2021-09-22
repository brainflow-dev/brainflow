#pragma once

#include <condition_variable>
#include <mutex>
#include <thread>

#include "bt_lib_board.h"


class Enophone : public BTLibBoard
{

protected:
    volatile bool keep_alive;
    bool is_streaming;
    std::thread streaming_thread;
    std::mutex m;
    std::condition_variable cv;
    volatile int state;

    void read_thread ();
    std::string get_name_selector ();

public:
    Enophone (struct BrainFlowInputParams params);
    ~Enophone ();

    int start_stream (int buffer_size, const char *streamer_params);
    int stop_stream ();
    int release_session ();
    int config_board (std::string config, std::string &response);
};
