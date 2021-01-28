#pragma once

#include <condition_variable>
#include <math.h>
#include <mutex>
#include <thread>

#include "board.h"
#include "board_controller.h"
#include "socket_server_udp.h"


class Fascia : public Board
{

private:
    const double accel_scale = 0.002 / (pow (2, 4));

    volatile bool keep_alive;
    bool initialized;
    bool is_streaming;
    std::thread streaming_thread;
    SocketServerUDP *socket;

    // mutex and cond variable to wait in the main thread for the first received package (this check
    // is optional)
    std::mutex m;
    std::condition_variable cv;
    volatile int state;

    void read_thread ();

public:
    Fascia (struct BrainFlowInputParams params);
    ~Fascia ();

    int prepare_session ();
    int start_stream (int buffer_size, char *streamer_params);
    int stop_stream ();
    int release_session ();
    int config_board (std::string config, std::string &response);

    static constexpr int package_size = 68;
    static constexpr int num_packages = 20;
    static constexpr int transaction_size = Fascia::package_size * Fascia::num_packages;
};
