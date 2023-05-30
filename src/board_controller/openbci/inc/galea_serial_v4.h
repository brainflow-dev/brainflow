#pragma once

#include <condition_variable>
#include <math.h>
#include <mutex>
#include <string>
#include <thread>

#include "board.h"
#include "board_controller.h"
#include "openbci_gain_tracker.h"
#include "serial.h"


class GaleaSerialV4 : public Board
{

private:
    volatile bool keep_alive;
    bool initialized;
    bool is_streaming;
    std::thread streaming_thread;
    Serial *serial;
    GaleaV4GainTracker gain_tracker;
    std::mutex m;
    std::condition_variable cv;
    volatile int state;
    volatile double half_rtt;

    void read_thread ();
    int calc_time (std::string &resp);


public:
    GaleaSerialV4 (struct BrainFlowInputParams params);
    ~GaleaSerialV4 ();

    int prepare_session ();
    int start_stream (int buffer_size, const char *streamer_params);
    int stop_stream ();
    int release_session ();
    int config_board (std::string config, std::string &response);
};