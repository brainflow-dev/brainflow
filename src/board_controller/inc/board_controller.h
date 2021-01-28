#pragma once

#include "board_info_getter.h" // include it here for matlab
#include "shared_export.h"


#ifdef __cplusplus
extern "C"
{
#endif
    // I dont use const char * because I am not sure that all
    // languages support passing const char * instead char *

    // data acquisition methods
    SHARED_EXPORT int CALLING_CONVENTION prepare_session (
        int board_id, char *json_brainflow_input_params);
    SHARED_EXPORT int CALLING_CONVENTION start_stream (
        int buffer_size, char *streamer_params, int board_id, char *json_brainflow_input_params);
    SHARED_EXPORT int CALLING_CONVENTION stop_stream (
        int board_id, char *json_brainflow_input_params);
    SHARED_EXPORT int CALLING_CONVENTION release_session (
        int board_id, char *json_brainflow_input_params);
    SHARED_EXPORT int get_current_board_data (int num_samples, double *data_buf,
        int *returned_samples, int board_id, char *json_brainflow_input_params);
    SHARED_EXPORT int CALLING_CONVENTION get_board_data_count (
        int *result, int board_id, char *json_brainflow_input_params);
    SHARED_EXPORT int CALLING_CONVENTION get_board_data (
        int data_count, double *data_buf, int board_id, char *json_brainflow_input_params);
    SHARED_EXPORT int CALLING_CONVENTION config_board (char *config, char *response,
        int *response_len, int board_id, char *json_brainflow_input_params);
    SHARED_EXPORT int CALLING_CONVENTION is_prepared (
        int *prepared, int board_id, char *json_brainflow_input_params);
    SHARED_EXPORT int CALLING_CONVENTION insert_marker (
        double marker_value, int board_id, char *json_brainflow_input_params);

    // logging methods
    SHARED_EXPORT int CALLING_CONVENTION set_log_level (int log_level);
    SHARED_EXPORT int CALLING_CONVENTION set_log_file (char *log_file);
    SHARED_EXPORT int CALLING_CONVENTION log_message (int log_level, char *message);

#ifdef __cplusplus
}
#endif