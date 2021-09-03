#pragma once

#include <condition_variable>
#include <mutex>
#include <thread>

#include "bt_lib_board.h"

#define ADS1299_Vref 4.5
#define ADS1299_gain 24.0


class Evomind : public BTLibBoard
{

    double eeg_scale = (double)(ADS1299_Vref / float ((pow (2, 23) - 1)) / ADS1299_gain * 1000000.);
    double accel_scale = (double)(0.002 / (pow (2, 4)));

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
    Evomind (struct BrainFlowInputParams params);
    ~Evomind ();

    int start_stream (int buffer_size, char *streamer_params);
    int stop_stream ();
    int release_session ();
    int config_board (std::string config, std::string &response);
};