#ifndef BOARDCONTROLLER
#define BOARDCONTROLLER

#ifdef _WIN32
    #define SHARED_EXPORT __declspec(dllexport)
#else
    #define SHARED_EXPORT
#endif

typedef enum
{
    STATUS_OK = 0,
    PORT_ALREADY_OPEN_ERROR,
    UNABLE_TO_OPEN_PORT_ERROR,
    SET_PORT_ERROR,
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

#ifdef __cplusplus
extern "C" {
#endif
SHARED_EXPORT int prepare_session (int board_id, char *port_name);
SHARED_EXPORT int start_stream (int buffer_size);
SHARED_EXPORT int stop_stream ();
SHARED_EXPORT int release_session ();
SHARED_EXPORT int get_current_board_data (int num_samples, float *data_buf, double *ts_buf, int *returned_samples);
SHARED_EXPORT int get_board_data_count (int *result);
SHARED_EXPORT int get_board_data (int data_count, float *data_buf, double *ts_buf);
SHARED_EXPORT int set_log_level (int log_level);
#ifdef __cplusplus
}
#endif

#endif
