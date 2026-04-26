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

    void read_thread ();

public:
    SyntheticBoard (struct BrainFlowInputParams params);
    ~SyntheticBoard () override;

    int prepare_session () override;
    int start_stream (int buffer_size, const char *streamer_params) override;
    int stop_stream () override;
    int release_session () override;
    int config_board (std::string config, std::string &response) override;
    int config_board_with_bytes (const char *bytes, int len) override;
};
