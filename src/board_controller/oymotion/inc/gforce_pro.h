#pragma once

#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>

#include "board.h"
#include "data_buffer.h"
#include "runtime_dll_loader.h"


class GforcePro : public Board
{

#ifdef _WIN32
private:
    static int num_objects;
    bool is_valid;

    volatile bool keep_alive;
    bool initialized;
    bool is_streaming;
    std::thread streaming_thread;

    int call_init ();
    int call_start ();
    int call_stop ();
    int call_release ();

    std::mutex m;
    std::condition_variable cv;
    volatile int state;

    void read_thread ();

    DLLLoader *dll_loader;
#endif

public:
    GforcePro (struct BrainFlowInputParams params);
    ~GforcePro ();

    int prepare_session ();
    int start_stream (int buffer_size, char *streamer_params);
    int stop_stream ();
    int release_session ();
    int config_board (std::string config, std::string &response);
};
