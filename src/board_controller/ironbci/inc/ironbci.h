#pragma once

#include <condition_variable>
#include <math.h>
#include <mutex>
#include <thread>

#include "board.h"
#include "board_controller.h"
#include "socket_client_tcp.h"
#include "socket_client_udp.h"


class IronBCI : public Board
{

private:
    volatile bool keep_alive;
    bool initialized;
    bool is_streaming;
    std::thread streaming_thread;
    SocketClientUDP *data_socket;
    SocketClientTCP *command_socket;

    std::mutex m;
    std::condition_variable cv;
    volatile int state;
    void read_thread ();

public:
    IronBCI (struct BrainFlowInputParams params);
    ~IronBCI ();

    int prepare_session ();
    int start_stream (int buffer_size, char *streamer_params);
    int stop_stream ();
    int release_session ();
    int config_board (std::string config, std::string &response);

    static constexpr int num_channels = 9;
    static constexpr int ads_gain = 8;
    static const std::string start_command_prefix; // command prefix to start streaming
    static const std::string stop_command;         // command which stops streaming
    static constexpr int package_size = 25;        // number of bytes in package
    static constexpr int num_packages = 10;        // number of packages in single udp transaction
    static constexpr int tcp_port = 2321;
    static constexpr int udp_port = 2322;
    static constexpr int transaction_size = IronBCI::package_size * IronBCI::num_packages;
};
