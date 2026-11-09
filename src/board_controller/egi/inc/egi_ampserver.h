#pragma once

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>

#include "board.h"
#include "socket_client_tcp.h"

class EGIAmpServer : public Board
{
    std::unique_ptr<SocketClientTCP> commands;
    std::unique_ptr<SocketClientTCP> data_socket;
    std::atomic<bool> keep_alive;
    std::atomic<int> stream_error;
    std::thread worker;
    std::mutex mutex;
    std::condition_variable received;
    bool ready;
    bool owns_stream;
    int amplifier;
    std::string details;
    bool send_all (SocketClientTCP &socket, const std::string &message);
    bool command (const std::string &name, const std::string &value, std::string &response);
    bool read_exact (void *data, size_t size);
    void read_thread ();

public:
    explicit EGIAmpServer (struct BrainFlowInputParams params);
    ~EGIAmpServer () override;
    int prepare_session () override;
    int start_stream (int buffer_size, const char *streamer_params) override;
    int stop_stream () override;
    int release_session () override;
    int config_board (std::string config, std::string &response) override;
};
