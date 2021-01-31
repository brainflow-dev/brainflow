#pragma once

#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>

#include "board.h"
#include "board_controller.h"
#include "data_buffer.h"
#include "runtime_dll_loader.h"


class Bitalino : public Board
{

private:
    static int num_objects;

    bool is_valid;

    std::string start_command;
    std::string stop_command;

    volatile bool keep_alive;
    bool initialized;
    bool is_streaming;
    std::thread streaming_thread;

    bool use_mac_addr;

    int call_init ();
    int call_open ();
    int call_close ();
    int call_start ();
    int call_stop ();
    int call_release ();

    std::mutex m;
    std::condition_variable cv;
    volatile int state;

    void read_thread ();

    DLLLoader *dll_loader;

public:
    Bitalino (struct BrainFlowInputParams params);
    ~Bitalino ();

    int prepare_session ();
    int start_stream (int buffer_size, char *streamer_params);
    int stop_stream ();
    int release_session ();
    int config_board (std::string config, std::string &response);
};
