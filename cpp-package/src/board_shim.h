#pragma once

#include "board_info_getter.h"
#include "brainflow_exception.h"

class BoardShim
{

    void reshape_data (int data_count, float *buf, double *ts_buf, double **output_buf);

public:
    int num_data_channels;
    int board_id;
    char port_name[1024];

    BoardShim (int board_id, const char *port_name);

    void prepare_session ();
    void start_stream (int buffer_size);
    void stop_stream ();
    void release_session ();
    void get_current_board_data (int num_samples, double **data_buf, int *returned_samples);
    void get_board_data_count (int *result);
    void get_board_data (int data_count, double **data_buf);
};
