#pragma once

#include <condition_variable>
#include <math.h>
#include <mutex>
#include <thread>

#include "board.h"
#include "board_controller.h"
#include "socket_client.h"

#define ADS1299_Vref 4.5
#define ADS1299_gain 24.0

class NovaXR : public Board
{

private:
    const float eeg_scale = ADS1299_Vref / float((pow (2, 23) - 1)) / ADS1299_gain * 1000000.;

    volatile bool keep_alive;
    bool initialized;
    bool is_streaming;
    std::thread streaming_thread;
    SocketClient *socket;

    std::mutex m;
    std::condition_variable cv;
    volatile int state;
    void read_thread ();

public:
    NovaXR (struct BrainFlowInputParams params);
    ~NovaXR ();

    int prepare_session ();
    int start_stream (int buffer_size);
    int stop_stream ();
    int release_session ();
    int config_board (char *config);

    static constexpr int package_size = 72;
    static constexpr int num_packages = 19;
    static constexpr int transaction_size = NovaXR::package_size * NovaXR::num_packages;
    static constexpr int num_channels = 25;
};
