#pragma once

#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>

#include "board.h"
#include "board_controller.h"
#include "data_buffer.h"
#include "runtime_dll_loader.h"


class Ganglion : public Board
{

private:
    static int num_objects;

    bool is_valid;

    double const accel_scale = 0.016f;
    double const eeg_scale = (1.2f * 1000000) / (8388607.0f * 1.5f * 51.0f);

    std::string start_command;
    std::string stop_command;

    volatile bool keep_alive;
    bool initialized;
    bool is_streaming;
    std::thread streaming_thread;

    bool use_mac_addr;
    int num_channels;

    // legacy from shared library, now we can do the same wo these helpers but lets keep it
    int call_init ();
    int call_config (char *config);
    int call_open ();
    int call_close ();
    int call_start ();
    int call_stop ();
    int call_release ();

    std::mutex m;
    std::condition_variable cv;
    volatile int state;

    void read_thread ();
    int start_streaming_prepared ();

    DLLLoader *dll_loader;

public:
    Ganglion (struct BrainFlowInputParams params);
    ~Ganglion ();

    int prepare_session ();
    int start_stream (int buffer_size, const char *streamer_params);
    int stop_stream ();
    int release_session ();
    int config_board (std::string config, std::string &response);
};
