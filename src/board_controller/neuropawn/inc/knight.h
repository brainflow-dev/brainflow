#pragma once

#include <thread>

#include "board.h"
#include "board_controller.h"
#include "serial.h"

class Knight : public Board
{

protected:
    volatile bool keep_alive;
    bool initialized;
    bool is_streaming;
    std::thread streaming_thread;
    Serial *serial;

    int min_package_size;

    virtual int send_to_board (const char *msg);
    virtual int send_to_board (const char *msg, std::string &response);
    virtual std::string read_serial_response ();
    int open_port ();
    int set_port_settings ();
    void read_thread ();

public:
    Knight (int board_id, struct BrainFlowInputParams params);
    ~Knight ();

    int prepare_session ();
    int start_stream (int buffer_size, const char *streamer_params);
    int stop_stream ();
    int release_session ();
    int config_board (std::string config, std::string &response);

    static constexpr int start_byte = 0xA0;
    static constexpr int end_byte = 0xC0;
};