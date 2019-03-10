#ifndef OPENBCI_BOARD
#define OPENBCI_BOARD

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <thread>

#include "board.h"
#include "spdlog/spdlog.h"
#include "data_buffer.h"
#include "board_controller.h"

#define MAX_CAPTURE_SAMPLES (86400*250)  // should be enough for one day of capturing


class OpenBCIBoard : public Board
{

    protected:

        volatile bool keep_alive;
        bool initialized;
        bool is_streaming;
        std::thread streaming_thread;

        char port_name[64]; // should be enought to store port name
        #ifdef _WIN32
        HANDLE port_descriptor;
        #else
        int port_descriptor;
        #endif

        DataBuffer *db;
        int num_channels;

        virtual int open_port ();
        virtual int send_to_board (char *message);
        virtual int status_check ();
        virtual int set_port_settings ();
        virtual void read_thread () = 0;

    public:
        OpenBCIBoard (int num_channels, const char *port_name);
        virtual ~OpenBCIBoard ();

        virtual int prepare_session ();
        virtual int start_stream (int buffer_size);
        virtual int stop_stream ();
        virtual int release_session ();
        int get_current_board_data (int num_samples, float *data_buf, double *ts_buf, int *returned_samples);
        int get_board_data_count (int *result);
        int get_board_data (int data_count, float *data_buf, double *ts_buf);

};

#endif
