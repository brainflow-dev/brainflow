#pragma once

#include <condition_variable>
#include <mutex>
#include <thread>

#include "bt_lib_board.h"


class Explore : public BTLibBoard
{

protected:
    volatile bool keep_alive;
    volatile int state;
    std::thread streaming_thread;
    std::mutex m;
    std::condition_variable cv;

    void read_thread ();
    std::string get_name_selector ();
    void parse_eeg_data (
        double *package, unsigned char *payload, int payload_size, double vref, int n_packages);
    void parse_orientation_data (double *package, unsigned char *payload, int payload_size);

public:
    Explore (int board_id, struct BrainFlowInputParams params);
    ~Explore ();

    int start_stream (int buffer_size, const char *streamer_params);
    int stop_stream ();
    int release_session ();
};
