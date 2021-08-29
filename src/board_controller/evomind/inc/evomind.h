#pragma once

#include <condition_variable>
#include <mutex>
#include <thread>

#include "board.h"
#include "board_controller.h"
#include "runtime_dll_loader.h"


class Evomind : public Board
{

private:
    volatile bool keep_alive;
    bool initialized;
    bool is_streaming;
    std::thread streaming_thread;
    DLLLoader *dll_loader;
    std::mutex m;
    std::condition_variable cv;
    volatile int state;

    int (*func_get_data) (char *, int, char *);

    void read_thread ();
    int call_start ();
    int call_stop ();

public:
    Evomind (struct BrainFlowInputParams params);
    ~Evomind ();

    int prepare_session ();
    int start_stream (int buffer_size, char *streamer_params);
    int stop_stream ();
    int release_session ();
    int config_board (std::string config, std::string &response);

protected:
    int find_evomind_addr ();
};
