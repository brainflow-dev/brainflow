#pragma once

#include <thread>

#include "board.h"
#include "board_controller.h"
#include "runtime_dll_loader.h"
#include "unicorn_types.h"


class UnicornBoard : public Board
{

private:
#if defined __linux__ || defined _WIN32
    UNICORN_HANDLE device_handle;

    volatile bool keep_alive;
    bool initialized;
    bool is_streaming;
    std::thread streaming_thread;
    // libunicorn.so requires specific linux distribution, to dont limit all supported boards to
    // these requirements load this library manually in runtime
    DLLLoader *dll_loader;
    // get_address can return an error to avoid error in acquisition thread, set this pointer in
    // prepare_session and store
    int (*func_get_data) (UNICORN_HANDLE, uint32_t, float *, uint32_t);

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
    int start_stream (int buffer_size, const char *streamer_params);
    int stop_stream ();
    int release_session ();
    int config_board (std::string config, std::string &response);

    static constexpr int package_size = 17; // from unicorn.h
};
