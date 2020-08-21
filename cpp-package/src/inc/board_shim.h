#pragma once

#include <cstdarg>
#include <string>

// include it here to allow user include only this single file
#include "board_controller.h"
#include "board_info_getter.h"
#include "brainflow_constants.h"
#include "brainflow_exception.h"
#include "brainflow_input_params.h"

/// LogLevels enum to store all possible log levels
enum class LogLevels : int
{
    LEVEL_TRACE = 0,    /// TRACE
    LEVEL_DEBUG = 1,    /// DEBUG
    LEVEL_INFO = 2,     /// INFO
    LEVEL_WARN = 3,     /// WARN
    LEVEL_ERROR = 4,    /// ERROR
    LEVEL_CRITICAL = 5, /// CRITICAL
    LEVEL_OFF = 6       // OFF
};


/// BoardShim class to communicate with a board
class BoardShim
{

    void reshape_data (int data_points, double *linear_buffer, double **output_buf);
    // can not init master_board_id in constructor cause we can not raise an exception from
    // constructor, also can not do it only in prepare_session cause it might not be a first called
    // method.
    int get_master_board_id ();
    std::string serialized_params;
    struct BrainFlowInputParams params;

public:
    // clang-format off
    /// disable BrainFlow loggers
    static void disable_board_logger ();
    /// enable BrainFlow logger with LEVEL_INFO
    static void enable_board_logger ();
    /// enable BrainFlow logger with LEVEL_TRACE
    static void enable_dev_board_logger ();
    /// redirect BrainFlow logger from stderr to file
    static void set_log_file (char *log_file);
    /// use set_log_level only if you want to write your own log messages to BrainFlow logger
    static void set_log_level (int log_level);
    /// write user defined string to BrainFlow logger
    static void log_message (int log_level, const char *format, ...);

    /**
     * get sampling rate for this board
     * @param board_id board id of your device
     * @throw BrainFlowException If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR
     */
    static int get_sampling_rate (int board_id);
    /**
     * get row index which holds package nums
     * @param board_id board id of your device
     * @throw BrainFlowException If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR
     */
    static int get_package_num_channel (int board_id);
    /**
     * get row index which holds timestamps
     * @param board_id board id of your device
     * @throw BrainFlowException If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR
     */
    static int get_timestamp_channel (int board_id);
    /**
     * get row index which holds battery level info
     * @param board_id board id of your device
     * @throw BrainFlowException If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR
     */
    static int get_battery_channel (int board_id);
    /**
     * get number of rows in returned from @ref get_board_data() 2d array
     * @param board_id board id of your device
     * @throw BrainFlowException If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR
     */
    static int get_num_rows (int board_id);
    /**
     * get eeg channel names in 10-20 system for devices with fixed electrode locations
     * @param board_id board id of your device
     * @throw BrainFlowException If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR
     */
    static std::string *get_eeg_names (int board_id, int *len);
    /**
     * get row indices which hold EEG data, for some board we can not split EEG\EMG\...
     * @param board_id board id of your device
     * @throw BrainFlowException If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR
     */
    static int *get_eeg_channels (int board_id, int *len);
    /**
     * get row indices which hold EMG data, for some board we can not split EEG\EMG\...
     * @param board_id board id of your device
     * @throw BrainFlowException If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR
     */
    static int *get_emg_channels (int board_id, int *len);
    /**
     * get row indices which hold ECG data, for some board we can not split EEG\EMG\...
     * @param board_id board id of your device
     * @throw BrainFlowException If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR
     */
    static int *get_ecg_channels (int board_id, int *len);
    /**
     * get row indices which hold EOG data, for some board we can not split EEG\EMG\...
     * @param board_id board id of your device
     * @throw BrainFlowException If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR
     */
    static int *get_eog_channels (int board_id, int *len);
    /**
     * get row indices which hold EXG data
     * @param board_id board id of your device
     * @throw BrainFlowException If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR
     */
    static int *get_exg_channels (int board_id, int *len);
    /**
     * get row indices which hold PPG data
     * @param board_id board id of your device
     * @throw BrainFlowException If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR
     */
    static int *get_ppg_channels (int board_id, int *len);
    /**
     * get row indices which hold EDA data
     * @param board_id board id of your device
     * @throw BrainFlowException If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR
     */
    static int *get_eda_channels (int board_id, int *len);
    /**
     * get row indices which hold accel data
     * @param board_id board id of your device
     * @throw BrainFlowException If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR
     */
    static int *get_accel_channels (int board_id, int *len);
    /**
     * get row indices which hold analog data
     * @param board_id board id of your device
     * @throw BrainFlowException If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR
     */
    static int *get_analog_channels (int board_id, int *len);
    /**
     * get row indices which hold gyro data
     * @param board_id board id of your device
     * @throw BrainFlowException If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR
     */
    static int *get_gyro_channels (int board_id, int *len);
    /**
     * get row indices which hold other information
     * @param board_id board id of your device
     * @throw BrainFlowException If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR
     */
    static int *get_other_channels (int board_id, int *len);
    /**
     * get row indices which hold temperature data
     * @param board_id board id of your device
     * @throw BrainFlowException If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR
     */
    static int *get_temperature_channels (int board_id, int *len);
    /**
     * get row indices which hold resistance data
     * @param board_id board id of your device
     * @throw BrainFlowException If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR
     */
    static int *get_resistance_channels (int board_id, int *len);

    int board_id;

    BoardShim (int board_id, struct BrainFlowInputParams params);
    ~BoardShim ()
    {
    }

    /// prepare BrainFlow's streaming session, should be called first
    void prepare_session ();
    /**
     * start streaming thread and store data in ringbuffer
     * @param buffer_size size of internal ring buffer
     * @param streamer_params use it to pass data packages further or store them directly during streaming,
                    supported values: "file://%file_name%:w", "file://%file_name%:a", "streaming_board://%multicast_group_ip%:%port%"".
                    Range for multicast addresses is from "224.0.0.0" to "239.255.255.255"
     */
    void start_stream (int buffer_size = 450000, char *streamer_params = NULL);
    /// check if session is ready or not
    bool is_prepared ();
    /// stop streaming thread, doesnt release other resources
    void stop_stream ();
    /// release streaming session
    void release_session ();
    /// get latest collected data, doesnt remove it from ringbuffer
    double **get_current_board_data (int num_samples, int *num_data_points);
    /// get number of packages in ringbuffer
    int get_board_data_count ();
    /// get all collected data and flush it from internal buffer
    double **get_board_data (int *num_data_points);
    /// send string to a board, use it carefully and only if you understand what you are doing
    void config_board (char *config);
    // clang-format on
};
