#pragma once

#include <thread>

#include "bluetooth_types.h"
#include "board.h"
#include "board_controller.h"
#include "runtime_dll_loader.h"


class Enophone : public Board
{

private:
    volatile BLUETOOTH_HANDLE bluetooth_handle;
    volatile bool keep_alive;
    bool initialized;
    bool is_streaming;
    std::thread streaming_thread;
    DLLLoader *dll_loader;

    void read_thread ();
    int call_start ();
    int call_stop ();

public:
    Enophone (struct BrainFlowInputParams params);
    ~Enophone ();

    int prepare_session ();
    int start_stream (int buffer_size, char *streamer_params);
    int stop_stream ();
    int release_session ();
    int config_board (std::string config, std::string &response);
};
