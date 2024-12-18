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

#define DEFAULT_CONTROL_PORT 12345
#define PACKET_DELIMITER_CSV '\n'

#define BIOLISTENER_COMMAND_UNDEFINED 0
#define BIOLISTENER_COMMAND_SET_ADC_DATA_RATE 1
#define BIOLISTENER_COMMAND_SET_ADC_CHANNEL_ENABLE 2
#define BIOLISTENER_COMMAND_SET_ADC_CHANNEL_PGA 3
#define BIOLISTENER_COMMAND_RESET_ADC 4
#define BIOLISTENER_COMMAND_START_SAMPLING 5
#define BIOLISTENER_COMMAND_STOP_SAMPLING 6

#define BIOLISTENER_DATA_PACKET_DEBUG 0
#define BIOLISTENER_DATA_PACKET_BIOSIGNALS 1
#define BIOLISTENER_DATA_PACKET_IMU 2

#define BIOLISTENER_DATA_PACKET_HEADER 0xA0
#define BIOLISTENER_DATA_PACKET_FOOTER 0xC0

#define BIOLISTENER_ADC_ADS131M08 0
#define BIOLISTENER_ADC_AD7771 1

#define BIOLISTENER_DATA_CHANNELS_COUNT 8
#define BIOLISTENER_DEFAULT_PGA_GAIN 8

#define FLOAT_TO_UINT32(x) (*((uint32_t *)&(x)))
#define UINT32_TO_FLOAT(x) (*((float *)&(x)))

typedef struct data_packet
{
    uint8_t header;
    uint32_t ts;
    uint8_t type;
    uint32_t n;
    uint8_t s_id;
    uint32_t data[BIOLISTENER_DATA_CHANNELS_COUNT];
    uint8_t footer;
} __attribute__ ((packed)) data_packet;

const size_t PACKET_SIZE = sizeof (data_packet);


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

    double timestamp_offset;

    std::mutex m_channels_gain;
    double channels_gain[BIOLISTENER_DATA_CHANNELS_COUNT] {0};

    void read_thread ();

    int create_control_connection ();
    int send_control_msg (const char *msg);
    int wait_for_connection ();

    static double data_to_volts (
        double ref, uint32_t raw_code, double pga_gain, double adc_resolution);

public:
    BioListener (int board_id, struct BrainFlowInputParams params);
    ~BioListener ();

    int prepare_session ();
    int start_stream (int buffer_size, const char *streamer_params);
    int stop_stream ();
    int release_session ();
    int config_board (std::string config, std::string &response);

    bool parse_tcp_buffer (const char *buffer, size_t buffer_size, data_packet &parsed_packet);
};
