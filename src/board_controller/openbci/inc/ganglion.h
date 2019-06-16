#ifndef GANGLION
#define GANGLION

#include <condition_variable>
#include <mutex>
#include <thread>

#include "board.h"
#include "board_controller.h"
#include "data_buffer.h"
#include "runtime_dll_loader.h"

#define MAX_CAPTURE_SAMPLES (86400 * 250) // should be enough for one day of capturing

class Ganglion : public Board
{

private:
    float const accel_scale = 0.032f;
    float const eeg_scale = 1.2f * 8388607.0f * 1.5f * 51.0f;

    volatile bool keep_alive;
    bool initialized;
    bool is_streaming;
    std::thread streaming_thread;

    char mac_addr[1024];
    bool use_mac_addr;
    int num_channels;
    DLLLoader *dll_loader;

    int call_init ();
    int call_open ();
    int call_close ();
    int call_start ();
    int call_stop ();

    /*
    at least for windows from time to time callback for value change notification is not triggered
    restart solves this issue, so if callback is not tiriggered we will reset Ganglion Device and
    wait in main thread for data
    */
    std::mutex m;
    std::condition_variable cv;
    volatile int state;

    void read_thread ();

public:
    Ganglion (const char *port_name);
    ~Ganglion ();

    int prepare_session ();
    int start_stream (int buffer_size);
    int stop_stream ();
    int release_session ();
    int get_current_board_data (
        int num_samples, float *data_buf, double *ts_buf, int *returned_samples);
    int get_board_data_count (int *result);
    int get_board_data (int data_count, float *data_buf, double *ts_buf);
    int get_board_id ()
    {
        return GANGLION_BOARD;
    }
};

#endif
