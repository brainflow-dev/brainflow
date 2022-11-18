#pragma once

#include <condition_variable>
#include <mutex>
#include <stdint.h>
#include <string>
#include <thread>

#include "board.h"
#include "board_controller.h"

#include "broadcast_server.h"
#include "socket_client_udp.h"
#include "socket_server_tcp.h"


class Emotibit : public Board
{

private:
    volatile bool keep_alive;
    volatile int state;

    std::string ip_address;
    bool initialized;
    std::thread streaming_thread;
    SocketClientUDP *data_socket;
    SocketServerTCP *control_socket;
    BroadCastServer *advertise_socket_server;
    std::mutex m;
    std::condition_variable cv;

    void read_thread ();

    std::string create_package (const std::string &type_tag, uint16_t packet_number,
        const std::string &data, uint16_t data_length, uint8_t protocol_version = 1,
        uint8_t data_reliability = 100);
    std::string create_package (const std::string &type_tag, uint16_t packet_number,
        const std::vector<std::string> &data, uint8_t protocol_version, uint8_t data_reliability);
    std::string create_header (const std::string &type_tag, uint32_t timestamp,
        uint16_t packet_number, uint16_t data_length, uint8_t protocol_version = 1,
        uint8_t data_reliability = 100);
    std::vector<std::string> split_string (const std::string &package, char delim);
    bool get_header (
        const std::string &package_string, int *package_num, int *data_len, std::string &type_tag);

    int create_adv_connection ();
    int create_data_connection ();
    int create_control_connection ();

public:
    Emotibit (struct BrainFlowInputParams params);
    ~Emotibit ();

    int prepare_session ();
    int start_stream (int buffer_size, const char *streamer_params);
    int stop_stream ();
    int release_session ();
    int config_board (std::string config, std::string &response);
};