#pragma once

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>

#include "board.h"
#include "serial.h"

class NeuroSky : public Board
{
    std::unique_ptr<Serial> serial;
    std::atomic<bool> keep_alive;
    std::thread worker;
    std::mutex mutex;
    std::condition_variable received;
    bool ready;
    void read_thread ();

public:
    explicit NeuroSky (struct BrainFlowInputParams params);
    ~NeuroSky () override;
    int prepare_session () override;
    int start_stream (int buffer_size, const char *streamer_params) override;
    int stop_stream () override;
    int release_session () override;
    int config_board (std::string config, std::string &response) override;
};
