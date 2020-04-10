#pragma once

#include "board_controller.h"
#include "brainflow_input_params.h"
#include "data_buffer.h"
#include "spdlog/spdlog.h"
#include "streamer.h"

#define MAX_CAPTURE_SAMPLES (86400 * 250) // should be enough for one day of capturing


class Board
{
public:
    static std::shared_ptr<spdlog::logger> board_logger;
    static int set_log_level (int log_level);
    static int set_log_file (char *log_file);

    virtual ~Board ()
    {
        skip_logs = true; // also should be set in inherited class destructor because it will be
                          // called before
        if (db != NULL)
        {
            delete db;
            db = NULL;
        }

        if (streamer != NULL)
        {
            delete streamer;
            streamer = NULL;
        }
    }
    Board (int board_id, struct BrainFlowInputParams params)
    {
        skip_logs = false;
        db = NULL;       // should be initialized in start_stream
        streamer = NULL; // should be initialized in start_stream
        this->board_id = board_id;
        this->params = params;
    }
    virtual int prepare_session () = 0;
    virtual int start_stream (int buffer_size, char *streamer_params) = 0;
    virtual int stop_stream () = 0;
    virtual int release_session () = 0;
    virtual int config_board (char *config) = 0;

    int get_current_board_data (int num_samples, double *data_buf, int *returned_samples);
    int get_board_data_count (int *result);
    int get_board_data (int data_count, double *data_buf);
    int prepare_streamer (char *streamer_params);

    // Board::board_logger should not be called from destructors, to ensure that there are safe log
    // methods Board::board_logger still available but should be used only outside destructors
    template <typename Arg1, typename... Args>
    void safe_logger (spdlog::level::level_enum log_level, const char *fmt, const Arg1 &arg1,
        const Args &... args)
    {
        if (!skip_logs)
        {
            Board::board_logger->log (log_level, fmt, arg1, args...);
        }
    }

    template <typename T> void safe_logger (spdlog::level::level_enum log_level, const T &msg)
    {
        if (!skip_logs)
        {
            Board::board_logger->log (log_level, msg);
        }
    }

protected:
    DataBuffer *db;
    bool skip_logs;
    int board_id;
    struct BrainFlowInputParams params;
    Streamer *streamer;

private:
    // reshapes data from DataBuffer format where all channels are mixed to linear buffer with
    // sorted data
    void reshape_data (int data_count, const double *buf, const double *ts_buf, double *output_buf);
};
