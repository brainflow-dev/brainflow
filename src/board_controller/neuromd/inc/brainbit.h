#pragma once

#include <thread>

#include "neuromd_board.h"

class BrainBit : public NeuromdBoard
{

private:
    // as for now BrainBit supports only windows and macos, to dont write ifdef in
    // board_controller.cpp add dummy implementation for linux
#if defined _WIN32 || defined __APPLE__
    volatile bool keep_alive;
    bool initialized;
    bool is_streaming;
    std::thread streaming_thread;
    // store data from callbacks as object fields and add it to eeg data
    volatile int last_battery;
    volatile double last_resistance_t3;
    volatile double last_resistance_t4;
    volatile double last_resistance_o1;
    volatile double last_resistance_o2;

    void read_thread ();

    void free_listeners ();
    void free_channels ();

    ListenerHandle battery_listener;
    ListenerHandle resistance_listener_t3;
    ListenerHandle resistance_listener_t4;
    ListenerHandle resistance_listener_o1;
    ListenerHandle resistance_listener_o2;
    SignalDoubleChannel *signal_t4;
    SignalDoubleChannel *signal_t3;
    SignalDoubleChannel *signal_o1;
    SignalDoubleChannel *signal_o2;
    volatile size_t counter;

#endif

public:
    BrainBit (struct BrainFlowInputParams params);
    ~BrainBit ();

    int prepare_session ();
    int start_stream (int buffer_size, const char *streamer_params);
    int stop_stream ();
    int release_session ();
    int config_board (std::string config, std::string &response);

#if defined _WIN32 || defined __APPLE__
    // callbacks must be public since they are called from plain C callbacks
    void on_battery_charge_received (
        Device *device, ChannelInfo channel_info, IntDataArray battery_data);
    void on_resistance_received (
        Device *device, ChannelInfo channel_info, DoubleDataArray data_array);

#endif
};
