#pragma once

#include <cmath>
#include <deque>
#include <limits>
#include <map>
#include <string>

#include "board_controller.h"
#include "brainflow_boards.h"
#include "brainflow_constants.h"
#include "brainflow_input_params.h"
#include "data_buffer.h"
#include "spinlock.h"
#include "streamer.h"

#include "spdlog/spdlog.h"

#define MAX_CAPTURE_SAMPLES (86400 * 250) // should be enough for one day of capturing


class Board
{
public:
    static std::shared_ptr<spdlog::logger> board_logger;
    static JNIEnv *java_jnienv; // nullptr unless on java
    static int set_log_level (int log_level);
    static int set_log_file (const char *log_file);

    virtual ~Board ()
    {
        skip_logs = true; // also should be set in inherited class destructor because it will be
                          // called before
        free_packages ();
    }

    Board (int board_id, struct BrainFlowInputParams params)
    {
        skip_logs = false;
        this->board_id = board_id;
        this->params = params;
        try
        {
            board_descr = boards_struct.brainflow_boards_json["boards"][std::to_string (board_id)];
        }
        catch (json::exception &e)
        {
            safe_logger (spdlog::level::err, e.what ());
        }
    }
    virtual int prepare_session () = 0;
    virtual int start_stream (int buffer_size, const char *streamer_params) = 0;
    virtual int stop_stream () = 0;
    virtual int release_session () = 0;
    virtual int config_board (std::string config, std::string &response) = 0;

    // some devices may implement it but there is no requirement to have this method and we do not
    // recommend anybody to use it
    virtual int config_board_with_bytes (const char *bytes, int len)
    {
        return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
    }

    int get_current_board_data (
        int num_samples, int preset, double *data_buf, int *returned_samples);
    int get_board_data_count (int preset, int *result);
    int get_board_data (int data_count, int preset, double *data_buf);
    int insert_marker (double value, int preset);
    int add_streamer (const char *streamer_params, int preset);
    int delete_streamer (const char *streamer_params, int preset);

    // Board::board_logger should not be called from destructors, to ensure that there are safe log
    // methods Board::board_logger still available but should be used only outside destructors
    template <typename Arg1, typename... Args>
    // clang-format off
    void safe_logger (
        spdlog::level::level_enum log_level, const char *fmt, const Arg1 &arg1, const Args &... args)
    // clang-format on
    {
        if (!skip_logs)
        {
            Board::board_logger->log (log_level, fmt, arg1, args...);
        }
    }

    template <typename T>
    void safe_logger (spdlog::level::level_enum log_level, const T &msg)
    {
        if (!skip_logs)
        {
            Board::board_logger->log (log_level, msg);
        }
    }

    int get_board_id ()
    {
        return board_id;
    }

protected:
    std::map<int, DataBuffer *> dbs;
    std::map<int, std::vector<Streamer *>> streamers;
    bool skip_logs;
    int board_id;
    struct BrainFlowInputParams params;
    json board_descr;
    SpinLock lock;
    std::map<int, std::deque<double>> marker_queues;

    int prepare_for_acquisition (int buffer_size, const char *streamer_params);
    void free_packages ();
    void push_package (double *package, int preset = (int)BrainFlowPresets::DEFAULT_PRESET);
    std::string preset_to_string (int preset);
    int preset_to_int (std::string preset);
    int parse_streamer_params (const char *streamer_params, std::string &streamer_type,
        std::string &streamer_dest, std::string &streamer_mods);

private:
    // reshapes data from DataBuffer format where all channels are mixed to linear buffer
    void reshape_data (int data_count, int preset, const double *buf, double *output_buf);
};
