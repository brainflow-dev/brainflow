#ifndef BOARDCONTROLLER
#define BOARDCONTROLLER

typedef enum
{
    STATUS_OK = 0,
    PORT_ALREADY_OPEN_ERROR,
    UNABLE_TO_OPEN_PORT_ERROR,
    SER_PORT_ERROR,
    BOARD_WRITE_ERROR,
    INCOMMING_MSG_ERROR,
    INITIAL_MSG_ERROR,
    BOARD_NOT_READY_ERROR,
    STREAM_ALREADY_RUN_ERROR,
    INVALID_BUFFER_SIZE_ERROR,
    STREAM_THREAD_ERROR,
    STREAM_THREAD_IS_NOT_RUNNING,
    EMPTY_BUFFER_ERROR,
    INVALID_ARGUMENTS_ERROR,
    UNSUPPORTED_BOARD_ERROR,
    BOARD_NOT_CREATED_ERROR
} CustomExitCodes;

typedef enum
{
    CYTHON_BOARD = 0,
    UNIMPLEMENTED
} BoardIds;

extern "C"
{
    int prepare_session (int board_id, const char *port_name, int is_dummy);
    int start_stream (int buffer_size);
    int stop_stream ();
    int release_session ();
    int get_current_board_data (int num_samples, float *data_buf, double *ts_buf, int *returned_samples);
    int get_board_data_count (int *result);
    int get_board_data (int data_count, float *data_buf, double *ts_buf);
}

#endif
