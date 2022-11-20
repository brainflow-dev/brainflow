#pragma once

#include <condition_variable>
#include <math.h>
#include <mutex>
#include <string>
#include <thread>

#include "board.h"
#include "board_controller.h"
#include "openbci_gain_tracker.h"
#include "socket_client_udp.h"


class Galea : public Board
{

private:
    volatile bool keep_alive;
    volatile int state;
    volatile double half_rtt;
    bool initialized;
    bool is_streaming;
    std::thread streaming_thread;
    SocketClientUDP *socket;
    std::mutex m;
    std::condition_variable cv;
    GaleaGainTracker gain_tracker;

    std::string find_device ();
    void read_thread ();
    int calc_time (std::string &resp);


public:
    Galea (struct BrainFlowInputParams params);
    ~Galea ();

    int prepare_session ();
    int start_stream (int buffer_size, const char *streamer_params);
    int stop_stream ();
    int release_session ();
    int config_board (std::string config, std::string &response);

    static constexpr int package_size = 72;
    static constexpr int max_num_packages = 25;
    static constexpr int max_transaction_size = package_size * max_num_packages;
    static constexpr int socket_timeout = 2;
};