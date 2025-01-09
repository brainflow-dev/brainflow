#pragma once

#include <string>
#include <thread>

#include "board.h"

#define EEGO_SDK_BIND_DYNAMIC

#include "eemagine/sdk/amplifier.h"
#include "eemagine/sdk/factory.h"
#include "eemagine/sdk/stream.h"
#include "eemagine/sdk/wrapper.h"


class AntNeuroBoard : public Board
{

#if defined __linux__ || defined _WIN32
private:
    volatile bool keep_alive;
    bool initialized;
    std::thread streaming_thread;
    std::string ant_neuro_lib_path;
    eemagine::sdk::factory *fact;
    eemagine::sdk::amplifier *amp;
    eemagine::sdk::stream *stream;
    int sampling_rate;
    double reference_range;
    double bipolar_range;
    bool impedance_mode;
    int impedance_package_num;

    void read_thread ();
#endif


public:
    AntNeuroBoard (int board_id, struct BrainFlowInputParams params);
    ~AntNeuroBoard ();

    int prepare_session ();
    int start_stream (int buffer_size, const char *streamer_params);
    int stop_stream ();
    int release_session ();
    int config_board (std::string config, std::string &response);
};
