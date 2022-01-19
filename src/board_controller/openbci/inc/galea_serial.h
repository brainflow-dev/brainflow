#pragma once

#include <condition_variable>
#include <math.h>
#include <mutex>
#include <string>
#include <thread>

#include "board.h"
#include "board_controller.h"
#include "serial.h"

#define ADS1299_Vref 4.5


class GaleaSerial : public Board
{

private:
    // different default gains
    const double eeg_scale_main_board = ADS1299_Vref / double ((pow (2, 23) - 1)) / 2.0 * 1000000.;
    const double eeg_scale_sister_board =
        ADS1299_Vref / double ((pow (2, 23) - 1)) / 12.0 * 1000000.;
    const double emg_scale = ADS1299_Vref / double ((pow (2, 23) - 1)) / 4.0 * 1000000.;

    volatile bool keep_alive;
    bool initialized;
    bool is_streaming;
    std::thread streaming_thread;
    Serial *serial;

    std::mutex m;
    std::condition_variable cv;
    volatile int state;
    volatile double half_rtt;
    void read_thread ();
    int calc_time (std::string &resp);


public:
    GaleaSerial (struct BrainFlowInputParams params);
    ~GaleaSerial ();

    int prepare_session ();
    int start_stream (int buffer_size, const char *streamer_params);
    int stop_stream ();
    int release_session ();
    int config_board (std::string config, std::string &response);
};
