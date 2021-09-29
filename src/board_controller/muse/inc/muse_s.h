#pragma once

#include "ble_lib_board.h"
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>


class MuseS : public BLELibBoard
{

private:
    volatile bool keep_alive;
    volatile simpleble_adapter_t muse_adapter;
    volatile simpleble_peripheral_t muse_peripheral;
    bool initialized;
    std::thread streaming_thread;
    std::mutex m;
    std::condition_variable cv;

    void read_thread ();

public:
    MuseS (struct BrainFlowInputParams params);
    ~MuseS ();

    int prepare_session ();
    int start_stream (int buffer_size, const char *streamer_params);
    int stop_stream ();
    int release_session ();
    int config_board (std::string config, std::string &response);

    void adapter_on_scan_found (simpleble_adapter_t adapter, simpleble_peripheral_t peripheral);
};
