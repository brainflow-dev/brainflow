#pragma once

#include <stdint.h>
#include <string>
#include <thread>

#include "board.h"
#include "board_controller.h"

#include "socket_client_udp.h"
#include "socket_server_tcp.h"


class Emotibit : public Board
{

private:
    volatile bool keep_alive;
    volatile bool initialized;

    std::string ip_address;
    std::thread streaming_thread;
    std::thread connection_thread;
    SocketClientUDP *data_socket;
    SocketServerTCP *control_socket;
    // there is one for broadcast but its used only to find ip, after
    // that its replaced by normal udp socket
    SocketClientUDP *adv_socket;
    std::mutex m;
    std::condition_variable cv;
    int control_port;
    int data_port;

    void read_thread ();
    void ping_thread ();

    std::string create_package (const std::string &type_tag, uint16_t packet_number,
        const std::string &data, uint16_t data_length, uint8_t protocol_version = 1,
        uint8_t data_reliability = 100);
    std::string create_package (const std::string &type_tag, uint16_t packet_number,
        std::vector<std::string> data, uint8_t protocol_version = 1,
        uint8_t data_reliability = 100);
    std::string create_header (const std::string &type_tag, uint32_t timestamp,
        uint16_t packet_number, uint16_t data_length, uint8_t protocol_version = 1,
        uint8_t data_reliability = 100);
    std::vector<std::string> split_string (const std::string &package, char delim);
    bool get_header (
        const std::string &package_string, int *package_num, int *data_len, std::string &type_tag);
    bool get_header (const std::string &package_string, int *package_num, int *data_len,
        std::string &type_tag, std::string &serial_number);
    std::vector<std::string> get_payload (const std::string &package_string, int data_len);

    int create_adv_connection ();
    int create_data_connection ();
    int create_control_connection ();
    int send_connect_msg ();
    int send_control_msg (const char *msg);
    int wait_for_connection ();

public:
    Emotibit (struct BrainFlowInputParams params);
    ~Emotibit ();

    int prepare_session ();
    int start_stream (int buffer_size, const char *streamer_params);
    int stop_stream ();
    int release_session ();
    int config_board (std::string config, std::string &response);
};