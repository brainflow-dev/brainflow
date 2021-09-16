#pragma once

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <string.h>
#include <string>
#include <thread>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "board.h"
#include "board_controller.h"
#include "runtime_dll_loader.h"


class DynLibBoard : public Board
{

protected:
    volatile bool keep_alive;
    bool initialized;
    bool is_streaming;
    std::thread streaming_thread;
    std::mutex m;
    std::condition_variable cv;
    volatile int state;
    DLLLoader *dll_loader;

    virtual int call_init ();
    virtual int call_open ();
    virtual int call_close ();
    virtual int call_start ();
    virtual int call_stop ();
    virtual int call_release ();
    virtual int call_config (char *config);

    virtual void read_thread ();
    virtual std::string get_lib_name () = 0;


public:
    DynLibBoard (int board_id, struct BrainFlowInputParams params);
    virtual ~DynLibBoard ();

    int prepare_session () override;
    int start_stream (int buffer_size, const char *streamer_params) override;
    int stop_stream () override;
    int release_session () override;
    int config_board (std::string config, std::string &response) override;
};
