#pragma once

#include <set>
#include <thread>

#include "board.h"
#include "board_controller.h"
#include "serial.h"

class KnightBase : public Board
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
    virtual int open_port ();
    virtual int set_port_settings ();
    virtual void read_thread () = 0;

private:
public:
    KnightBase (int board_id, struct BrainFlowInputParams params);
    ~KnightBase () override;

    int prepare_session () override;
    int start_stream (int buffer_size, const char *streamer_params) override;
    int stop_stream () override;
    int release_session () override;
    int config_board (std::string config, std::string &response) override;

    static constexpr int start_byte = 0xA0;
    static constexpr int end_byte = 0xC0;
};
