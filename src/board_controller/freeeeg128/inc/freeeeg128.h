#pragma once

#include <thread>

#include "board.h"
#include "board_controller.h"
#include "serial.h"


class FreeEEG128 : public Board
{

protected:
    volatile bool keep_alive;
    bool initialized;
    bool is_streaming;
    std::thread streaming_thread;
    Serial *serial;

    int open_port ();
    int set_port_settings ();
    void read_thread ();

public:
    FreeEEG128 (struct BrainFlowInputParams params);
    ~FreeEEG128 ();

    int prepare_session ();
    int start_stream (int buffer_size, const char *streamer_params);
    int stop_stream ();
    int release_session ();
    int config_board (std::string config, std::string &response);

    static constexpr int start_byte = 0xA0;
    static constexpr int end_byte = 0xC0;
    static constexpr double ads_gain = 8.0;
    static constexpr double ads_vref = 2.5;
};
