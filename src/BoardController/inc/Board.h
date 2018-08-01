#ifndef BOARD
#define BOARD

#include <thread>
#include <termios.h>

#include "spdlog/spdlog.h"
#include "DataBuffer.h"
#include "BoardController.h"

#define MAX_CAPTURE_SAMPLES (7*86400*250)  // should be enough for one week of capture


class Board
{

    protected:

        int is_dummy;
        volatile bool keep_alive;
        bool initialized;
        bool is_streaming;
        std::thread streaming_thread;
        
        char port_name[64]; // should be enought to store port name
        int port_descriptor;
        struct termios port_settings;

        DataBuffer *db;
        int num_channels;

        virtual int open_port ();
        virtual int send_to_board (char *message);
        virtual int status_check ();
        virtual int set_port_settings ();
        virtual void read_thread () = 0;

    public:
        Board (int num_channels, const char *port_name, int is_dummy);
        virtual ~Board ();

        virtual int prepare_session ();
        virtual int start_stream (int buffer_size);
        virtual int stop_stream ();
        virtual int release_session ();
        int get_current_board_data (int num_samples, float *data_buf, double *ts_buf, int *returned_samples);
        int get_board_data_count (int *result);
        int get_board_data (int data_count, float *data_buf, double *ts_buf);

        std::shared_ptr<spdlog::logger> logger;

};

#endif
