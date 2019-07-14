#pragma once

#include <thread>

#include "board.h"
#include "board_controller.h"


class SyntheticBoard : public Board
{

private:
    volatile bool keep_alive;
    bool initialized;
    bool is_streaming;
    std::thread streaming_thread;

    int num_channels;
    float amplitude;
    float shift;
    float noise;
    int sampling_rate;

    void read_thread ();

public:
    SyntheticBoard (const char *json_config);
    ~SyntheticBoard ();

    int prepare_session ();
    int start_stream (int buffer_size);
    int stop_stream ();
    int release_session ();
    int get_board_id ()
    {
        return SYNTHETIC_BOARD;
    }
};
