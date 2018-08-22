#ifndef BOARD_SHIM
#define BOARD_SHIM

class BoardShim
{

    void reshape_data (int data_count, float *buf, double *ts_buf, double **output_buf);

    public:
        int num_channels;
        int board_id;
        char port_name[1024];

        BoardShim (int board_id, const char *port_name);

        int prepare_session ();
        int start_stream (int buffer_size);
        int stop_stream ();
        int release_session ();
        int get_current_board_data (int num_samples, double **data_buf, int *returned_samples);
        int get_board_data_count (int *result);
        int get_board_data (int data_count, double **data_buf);
};

#endif