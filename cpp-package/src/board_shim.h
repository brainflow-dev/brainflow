#pragma once

#include <cstdarg>
#include <string>

// include it here to allow user include only this single file
#include "board_controller.h"
#include "brainflow_constants.h"
#include "brainflow_exception.h"


enum class LogLevels : int
{
    LEVEL_TRACE = 0,
    LEVEL_DEBUG = 1,
    LEVEL_INFO = 2,
    LEVEL_WARN = 3,
    LEVEL_ERROR = 4,
    LEVEL_CRITICAL = 5,
    LEVEL_OFF = 6
};

class BoardShim
{

    void reshape_data (int data_points, double *linear_buffer, double **output_buf);
    std::string input_params;

public:
    // logging methods
    static void disable_board_logger ();
    static void enable_board_logger ();
    static void enable_dev_board_logger ();
    static void set_log_file (char *log_file);
    // use set_log_level and log_message only if you want to write your own log messages to
    // brainflow logger
    static void set_log_level (int log_level);
    static void log_message (int log_level, const char *format, ...);

    // data desc and board desc methods, these methods return column indexes in data table
    static int get_sampling_rate (int board_id);
    static int get_package_num_channel (int board_id);
    static int get_timestamp_channel (int board_id);
    static int get_num_rows (int board_id);
    static int *get_eeg_channels (int board_id, int *len);
    static int *get_emg_channels (int board_id, int *len);
    static int *get_ecg_channels (int board_id, int *len);
    static int *get_eog_channels (int board_id, int *len);
    static int *get_ppg_channels (int board_id, int *len);
    static int *get_eda_channels (int board_id, int *len);
    static int *get_accel_channels (int board_id, int *len);
    static int *get_analog_channels (int board_id, int *len);
    static int *get_gyro_channels (int board_id, int *len);
    static int *get_other_channels (int board_id, int *len);

    int board_id;

    BoardShim (int board_id, struct BrainFlowInputParams params);
    ~BoardShim ()
    {
    }

    // data acquisition methods
    void prepare_session ();
    void start_stream (int buffer_size = 450000);
    void stop_stream ();
    void release_session ();
    double **get_current_board_data (int num_samples, int *num_data_points);
    int get_board_data_count ();
    double **get_board_data (int *num_data_points);
    void config_board (char *config);
};
