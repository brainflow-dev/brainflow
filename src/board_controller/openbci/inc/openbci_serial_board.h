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
    int current_sampling_rate;
    std::thread streaming_thread;

    Serial *serial;

    virtual int open_port ();
    virtual int status_check ();
    virtual int set_port_settings ();
    virtual void read_thread () = 0;
    virtual int send_to_board (const char *msg);
    virtual int send_to_board (const char *msg, std::string &response);
    virtual std::string read_serial_response ();

public:
    OpenBCISerialBoard (struct BrainFlowInputParams params, int board_id);
    ~OpenBCISerialBoard () override;

    int prepare_session () override;
    int start_stream (int buffer_size, const char *streamer_params) override;
    int stop_stream () override;
    int release_session () override;
    int config_board (std::string config, std::string &response) override;
    int get_board_sampling_rate (int preset) override;
};
