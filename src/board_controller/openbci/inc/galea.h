#pragma once

#include <condition_variable>
#include <math.h>
#include <mutex>
#include <string>
#include <thread>

#include "board.h"
#include "board_controller.h"
#include "socket_client_udp.h"

#define ADS1299_Vref 4.5


class Galea : public Board
{

private:
    // different default gains
    const double eeg_scale_main_board = ADS1299_Vref / double ((pow (2, 23) - 1)) / 2.0 * 1000000.;
    const double eeg_scale_sister_board =
        ADS1299_Vref / double ((pow (2, 23) - 1)) / 12.0 * 1000000.;
    const double emg_scale = ADS1299_Vref / double ((pow (2, 23) - 1)) / 4.0 * 1000000.;

    volatile bool keep_alive;
    volatile int state;
    volatile double half_rtt;
    bool initialized;
    bool is_streaming;
    std::thread streaming_thread;
    SocketClientUDP *socket;
    std::mutex m;
    std::condition_variable cv;

    std::string find_device ();
    void read_thread ();
    int calc_time (std::string &resp);
    void add_exg_package (
        double *package, unsigned char *bytes, double pc_timestamp, DataBuffer *times);
    void add_aux_package (
        double *package, unsigned char *bytes, double pc_timestamp, DataBuffer *times);


public:
    Galea (struct BrainFlowInputParams params);
    ~Galea ();

    int prepare_session ();
    int start_stream (int buffer_size, const char *streamer_params);
    int stop_stream ();
    int release_session ();
    int config_board (std::string config, std::string &response);

    static constexpr int max_bytes_in_transaction = 8192;
    static constexpr int exg_package_size = 59;
    static constexpr int num_exg_packages = 20;
    static constexpr int exg_transaction_size = exg_package_size * num_exg_packages;
    static constexpr int aux_package_size = 26;
    static constexpr int num_aux_packages = 4;
    static constexpr int aux_transaction_size = aux_package_size * num_aux_packages;
    static constexpr int socket_timeout = 2;
};
