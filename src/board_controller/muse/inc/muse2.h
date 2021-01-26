#pragma once

#include <future>

#include "board.h"
#include "board_controller.h"

#include "bluetooth.h"

class Muse2 : public Board
{

private:
    volatile bool keep_alive;
    bool initialized;
    bool is_streaming;
    std::thread streaming_thread;

    BluetoothLE bt;

    std::mutex serial_data_lock;
    std::string serial_data_needslock;
    std::promise<nlohmann::json> serial_reply;
    void serial_on_response (const uint8_t *data, size_t len);

    nlohmann::json serial_write_throws (std::string cmd); // cmd not constref because mutated

    void read_thread ();

public:
    Muse2 (struct BrainFlowInputParams params);
    ~Muse2 ();

    int prepare_session ();
    int start_stream (int buffer_size, char *streamer_params);
    int stop_stream ();
    int release_session ();
    int config_board (std::string config, std::string &response);
};
