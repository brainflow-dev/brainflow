#pragma once

#include <string>
#include <thread>

#include "board.h"
#include "board_controller.h"
#include "math.h"
#include "socket_server_tcp.h"

#ifdef USE_PERIPHERY
#include "gpio.h"
#include "spi.h"
#endif

class PIEEGBoard : public Board
{
protected:
#ifdef USE_PERIPHERY
    volatile bool keep_alive;
    bool initialized;
    std::thread streaming_thread;
    spi_t *spi;
    gpio_t *gpio_in;
    SocketServerTCP *server_socket;
    void read_thread ();
    int write_reg (uint8_t reg_address, uint8_t val);
    int send_command (uint8_t command);
#endif

public:
    PIEEGBoard (int board_id, struct BrainFlowInputParams params);
    ~PIEEGBoard ();

    int prepare_session ();
    int start_stream (int buffer_size, const char *streamer_params);
    int stop_stream ();
    int release_session ();
    int config_board (std::string config, std::string &response);
};
