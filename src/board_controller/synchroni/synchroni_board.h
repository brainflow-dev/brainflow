#pragma once


#include "board.h"
#include "board_controller.h"
#include "ble_lib_board.h"

class SynchroniBoard : public BLELibBoard
{

private:


public:
    SynchroniBoard (struct BrainFlowInputParams params);
    ~SynchroniBoard ();

    int prepare_session ();
    int start_stream (int buffer_size, const char *streamer_params);
    int stop_stream ();
    int release_session ();
    int config_board (std::string config, std::string &response);
    int config_board (std::string config);
};