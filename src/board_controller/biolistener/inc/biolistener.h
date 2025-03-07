#pragma once

#include <stdint.h>
#include <string>
#include <thread>

#include "board.h"
#include "board_controller.h"

#include "broadcast_server.h"
#include "socket_client_udp.h"
#include "socket_server_tcp.h"
#include "socket_server_udp.h"


#pragma pack(push, 1)
template <size_t BIOLISTENER_DATA_CHANNELS_COUNT>
struct data_packet
{
    uint8_t header;
    uint32_t ts;
    uint8_t type;
    uint32_t n;
    uint8_t s_id;
    uint32_t data[BIOLISTENER_DATA_CHANNELS_COUNT];
    uint8_t footer;
};
#pragma pack(pop)


template <size_t BIOLISTENER_DATA_CHANNELS_COUNT>
class BioListener : public Board
{

private:
    volatile bool keep_alive;
    volatile bool initialized;

    std::string ip_address;
    std::thread streaming_thread;
    SocketServerTCP *control_socket;
    std::mutex m;
    std::condition_variable cv;
    int control_port;
    int data_port;

    size_t packet_size;

    double timestamp_offset;

    std::mutex m_channels_gain;
    double channels_gain[BIOLISTENER_DATA_CHANNELS_COUNT] {0};

    void read_thread ();

    int create_control_connection ();
    int send_control_msg (const char *msg);
    int wait_for_connection ();

    static double data_to_volts (
        double ref, uint32_t raw_code, double pga_gain, double adc_resolution);

    bool parse_tcp_buffer (const char *buffer, size_t buffer_size,
        data_packet<BIOLISTENER_DATA_CHANNELS_COUNT> &parsed_packet);

public:
    BioListener (int board_id, struct BrainFlowInputParams params);
    ~BioListener ();

    int prepare_session ();
    int start_stream (int buffer_size, const char *streamer_params);
    int stop_stream ();
    int release_session ();
    int config_board (std::string config, std::string &response);
};
