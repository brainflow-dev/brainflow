#pragma once

#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

#include "board.h"
#include "board_controller.h"


class PlaybackFileBoard : public Board
{

private:
    volatile bool keep_alive;
    volatile bool loopback;
    volatile bool use_new_timestamps;
    std::vector<double> pos_percentage;
    std::vector<std::thread> streaming_threads;
    bool initialized;
    std::vector<std::vector<long int>> file_offsets;

    int validate_file_access (const std::string &filename);
    void read_thread (int preset, std::string filename);
    int get_file_offsets (std::string filename, std::vector<long int> &offsets);

public:
    PlaybackFileBoard (struct BrainFlowInputParams params);
    ~PlaybackFileBoard () override;

    int prepare_session () override;
    int start_stream (int buffer_size, const char *streamer_params) override;
    int stop_stream () override;
    int release_session () override;
    int config_board (std::string config, std::string &response) override;
};
