#pragma once

#include <string>
#include <thread>

#include "ble_lib_board.h"


class MuseS : public BLELibBoard
{

private:
    volatile bool keep_alive;
    bool initialized;
    std::thread streaming_thread;

    void read_thread ();

public:
    MuseS (struct BrainFlowInputParams params);
    ~MuseS ();

    int prepare_session ();
    int start_stream (int buffer_size, char *streamer_params);
    int stop_stream ();
    int release_session ();
    int config_board (std::string config, std::string &response);
};
