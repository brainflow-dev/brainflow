#pragma once

#include <condition_variable>
#include <mutex>
#include <thread>

#include "board.h"
#include "board_controller.h"
#include "data_buffer.h"
#include "runtime_dll_loader.h"


class Ganglion : public Board
{

private:
    float const accel_scale = 0.032f;
    float const eeg_scale = (1.2f * 1000000) / (8388607.0f * 1.5f * 51.0f);

    volatile bool keep_alive;
    bool initialized;
    bool is_streaming;
    std::thread streaming_thread;

    char mac_addr[1024];
    bool use_mac_addr;
    int num_channels;
    DLLLoader *dll_loader;

    int call_init ();
    int call_config (char *config);
    int call_open ();
    int call_close ();
    int call_start ();
    int call_stop ();

    /*
    at least for windows from time to time callback for value change notification is not triggered
    restart solves this issue, so if callback is not tiriggered we will reset Ganglion Device and
    wait in main thread for data
    */
    std::mutex m;
    std::condition_variable cv;
    volatile int state;

    void read_thread ();

public:
    Ganglion (const char *port_name);
    ~Ganglion ();

    int prepare_session ();
    int start_stream (int buffer_size);
    int stop_stream ();
    int release_session ();
    int config_board (char *config);
    int get_board_id ()
    {
        return GANGLION_BOARD;
    }
};
