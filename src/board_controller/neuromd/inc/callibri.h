#pragma once

#include <thread>

#include "neuromd_board.h"

class Callibri : public NeuromdBoard
{

#if defined _WIN32 || defined __APPLE__

protected:
    // different settings for EEG/ECG/EMG, currently we change only sampling rate and can pass it as
    // an int value but for future lets have a function defined per class
    virtual int apply_initial_settings () = 0;

private:
    volatile bool keep_alive;
    bool initialized;
    bool is_streaming;
    std::thread streaming_thread;
    volatile size_t counter;

    void read_thread ();
    void free_channels ();

    SignalDoubleChannel *signal_channel;

#endif

public:
    Callibri (int board_id, struct BrainFlowInputParams params);
    ~Callibri ();

    int prepare_session ();
    int start_stream (int buffer_size, const char *streamer_params);
    int stop_stream ();
    int release_session ();
    int config_board (std::string config, std::string &response);
};
