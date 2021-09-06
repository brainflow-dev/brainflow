#pragma once

#include <string>
#include <thread>

#include "board.h"
#include "board_controller.h"
#include "math.h"

#ifdef USE_PERIPHERY
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

    void read_thread ();
#endif

public:
    IronBCI (struct BrainFlowInputParams params);
    ~IronBCI ();

    int prepare_session ();
    int start_stream (int buffer_size, char *streamer_params);
    int stop_stream ();
    int release_session ();
    int config_board (std::string config, std::string &response);

    static constexpr int ads_gain = 8;
    static constexpr int start_byte = 0xA0; // package start byte
    static constexpr int stop_byte = 0xC0;  // package stop byte
    static const std::string start_command; // command which starts streaming
    static const std::string stop_command;  // command which stops streaming
};
