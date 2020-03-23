#pragma once

#include <thread>

#include "board.h"
#include "board_controller.h"
#include "runtime_dll_loader.h"


class UnicornBoard : public Board
{

private:
#ifdef __linux__
    volatile bool keep_alive;
    bool initialized;
    bool is_streaming;
    std::thread streaming_thread;
    DLLLoader *dll_loaderer;

    void read_thread ();

    int call_open ();
    int call_close ();
    int call_start ();
    int call_stop ();
#endif

public:
    UnicornBoard (struct BrainFlowInputParams params);
    ~UnicornBoard ();

    int prepare_session ();
    int start_stream (int buffer_size, char *streamer_params);
    int stop_stream ();
    int release_session ();
    int config_board (char *config);

    static constexpr int package_size = 17; // from unicorn.h
};