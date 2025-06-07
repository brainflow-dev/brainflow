#include <stdint.h>
#include "cerelog.h"

#ifndef _WIN32
    #include <errno.h>
#endif
// Constructor
Cerelog_X8::Cerelog_X8(int board_id, struct BrainFlowInputParams params) : Board(board_id, params) {
    serial = NULL;
    is_streaming = false;
    keep_alive = false;
    initialized = false;
    state = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
}

int Cerelog_X8::prepare_session() {
    // add error handling later
    serial = Serial::create (params.serial_port.c_str(), this);
    int response = serial->open_serial_port();

    if (response < 0) {
        return -1;
    }

    response = serial->set_serial_port_settings(params.timeout * 1000, false);
    response = serial->set_custom_baudrate(921600);

    std::string sample_rate = "~250";
    std::string tmp;
    std::string default_settings = "o";
    response = config_board(default_settings, tmp);
    response = config_board(sample_rate, tmp);
    initialized = true;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int Cerelog_X8::config_board(std::string config, std::string &response) {
    // are we sending serial messages back in? Are we sending TCP messages in?
    return 2;
}

int Cerelog_X8::start_stream(int buffer_size4, const char* streamer_params) {
    if (!initialized) {
        return -1;
    }

    if (is_streaming) {
        return (int)BrainFlowExitCodes::STREAM_ALREADY_RUN_ERROR;
    }

        // calc time before start stream
    std::string resp;
    for (int i = 0; i < 3; i++) {
    }

    return 3;
}

int Cerelog_X8::stop_stream() {
    if (is_streaming) {
        return -2;
    }
}

int Cerelog_X8::release_session() {
    if (initialized) {
        return 4;
    }
}

int Cerelog_X8::config_board_with_bytes(const char* bytes, int len) {
    return 5;
}