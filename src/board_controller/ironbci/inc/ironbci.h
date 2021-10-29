#pragma once

#include <string>
#include <thread>

#include "board.h"
#include "board_controller.h"
#include "math.h"

#ifdef USE_PERIPHERY
#include "gpio.h"
#include "spi.h"
#endif


class IronBCI : public Board
{

protected:
#ifdef USE_PERIPHERY
    volatile bool keep_alive;
    bool initialized;
    std::thread streaming_thread;
    spi_t *spi;
    gpio_t *gpio_in;

    void read_thread ();
    int write_reg (uint8_t reg_address, uint8_t val);
    int send_command (uint8_t command);
#endif

public:
    IronBCI (struct BrainFlowInputParams params);
    ~IronBCI ();

    int prepare_session ();
    int start_stream (int buffer_size, const char *streamer_params);
    int stop_stream ();
    int release_session ();
    int config_board (std::string config, std::string &response);
};