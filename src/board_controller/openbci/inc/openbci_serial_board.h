#pragma once

#include <thread>

#include "board.h"
#include "board_controller.h"
#include "serial.h"


class OpenBCISerialBoard : public Board
{

protected:
    volatile bool keep_alive;
    bool initialized;
    bool is_streaming;
    std::thread streaming_thread;

    Serial *serial;
    int num_channels;

    virtual int open_port ();
    virtual int status_check ();
    virtual int set_port_settings ();
    virtual void read_thread () = 0;
    virtual int send_to_board (char *msg);

public:
    OpenBCISerialBoard (int num_channels, struct BrainFlowInputParams params, int board_id);
    virtual ~OpenBCISerialBoard ();

    virtual int prepare_session ();
    virtual int start_stream (int buffer_size, char *streamer_params);
    virtual int stop_stream ();
    virtual int release_session ();
    virtual int config_board (char *config);
};
