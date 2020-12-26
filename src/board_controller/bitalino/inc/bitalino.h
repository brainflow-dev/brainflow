#pragma once

#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>

#include "board.h"
#include "board_controller.h"
#include "data_buffer.h"
#include "runtime_dll_loader.h"


class BITalino : public Board 
{

private:
    static int num_objects;

    bool is_valid;

    float const accel_scale = 0.016f;
//__________________________ EEG Scale for BITalino sensor ______________//
	// [-41.25uv , 41.25uv]
    float const vcc = 3.3;        // operating voltage (v)
    float const Geeg = 40000; // sensor gain
    float const n = 10;           // channel number, in this case is 10,(1024)
    float const eegMP = float(0.5 * vcc) / Geeg;
    float const eeg_scale = float(vcc) / (1024 * Geeg);
    float const uvolt = 1000000;
    //__________________________ ECG Scale for BITalino sensor ______________//
    // [-1.5mv , 1.5mv]
    float const Gecg = 1100; // sensor gain
    float const ecgMP = float(0.5 * vcc) / Gecg;
    float const ecg_scale = float(vcc) / (1024 * Gecg);
    float const mvolt = 1000;

    std::string start_command;
    std::string stop_command;
	
    volatile bool keep_alive;
    bool initialized;
    bool is_streaming;
    std::thread streaming_thread;

    bool use_mac_addr;
    int num_channels;

    // legacy from shared library, now we can do the same wo these helpers but lets keep it
    int call_init ();
    int call_config (char *config);
    int call_open ();
    int call_close ();
    int call_start ();
    int call_stop ();
    int call_release ();

    std::mutex m;
    std::condition_variable cv;
    volatile int state;

    void read_thread ();
    int start_streaming_prepared ();

    DLLLoader *dll_loader;

public:
    BITalino (struct BrainFlowInputParams params);
    ~BITalino (); 

    int prepare_session ();
    int start_stream (int buffer_size, char *streamer_params);
    int stop_stream ();
    int release_session ();
    //int config_board (char *config);
    int config_board (std::string config, std::string &response);
};
