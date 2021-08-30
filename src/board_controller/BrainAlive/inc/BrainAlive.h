#pragma once

#include <condition_variable>
#include <mutex>
#include <thread>

#include "board.h"
#include "board_controller.h"
#include "runtime_dll_loader.h"
typedef __declspec(dllimport) int (__cdecl *DLLFunc) (LPVOID);
class BrainAlive_Device : public Board
{
private:
    bool is_valid;
    bool use_mac_addr;
    volatile bool keep_alive;
    bool initialized;
    bool is_streaming;
    std::thread streaming_thread;
    DLLLoader *dll_loader;
    std::mutex m;
    std::condition_variable cv;
 
    int (*func_get_data) (void *);
    volatile int state;
  
    void read_thread ();
    int call_start ();
    int call_stop ();

public:
    BrainAlive_Device (struct BrainFlowInputParams params);
    ~BrainAlive_Device ();
   
    int prepare_session ();
    int start_stream (int buffer_size, char *streamer_params);
    int stop_stream ();
    int release_session ();
    int config_board (std::string config, std::string &response);

 

};
