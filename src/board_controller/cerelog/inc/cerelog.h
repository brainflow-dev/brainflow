#pragma once

#include <thread>

#include "board.h"
#include "board_controller.h"
#include "serial.h"

class Cerelog_X8 : public Board {    
    private:
        volatile bool keep_alive;
        bool initialized;
        bool is_streaming;
        std::thread streaming_thread;
        Serial *serial;
        int state;
        std::mutex m; // This is for thread processing later on
        std::condition_variable cv; // I don't really know what this is doing

        void read_thread();

    public:
        Cerelog_X8(int board_id, struct BrainFlowInputParams params);
        int prepare_session();
        int start_stream(int buffer_size, const char *streamer_params);
        int stop_stream();
        int release_session();
        int config_board(std::string config, std::string &response);
        int config_board_with_bytes (const char *bytes, int len);
};