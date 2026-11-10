#pragma once

#include <atomic>
#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <thread>

#include "board.h"
#include "rda_protocol.h"
#include "socket_client_tcp.h"

class BrainVisionRDA : public Board
{
    std::unique_ptr<SocketClientTCP> socket;
    std::atomic<bool> keep_alive;
    std::atomic<int> stream_error;
    std::thread worker;
    std::mutex mutex;
    std::condition_variable received;
    bool initialized;
    bool ready;
    std::vector<std::string> channel_names;
    std::deque<rda::Marker> markers;
    bool connect_socket ();
    bool read_exact (void *data, size_t size);
    void read_thread ();

public:
    explicit BrainVisionRDA (struct BrainFlowInputParams params);
    ~BrainVisionRDA () override;
    int prepare_session () override;
    int start_stream (int buffer_size, const char *streamer_params) override;
    int stop_stream () override;
    int release_session () override;
    int config_board (std::string config, std::string &response) override;
};
