#pragma once

#include "data_buffer.h"
#include "spdlog/spdlog.h"

#define MAX_CAPTURE_SAMPLES (86400 * 250) // should be enough for one day of capturing


class Board
{
public:
    static spdlog::logger *board_logger;
    static int set_log_level (int level);

    virtual ~Board ()
    {
        skip_logs = true; // also should be set in inherited class destructor because it will be
                          // called before
        if (db != NULL)
        {
            delete db;
            db = NULL;
        }
    }
    Board ()
    {
        skip_logs = false;
        db = NULL; // should be initialized in start_stream
    }
    virtual int prepare_session () = 0;
    virtual int start_stream (int buffer_size) = 0;
    virtual int stop_stream () = 0;
    virtual int release_session () = 0;
    virtual int config_board (char *config) = 0;

    int get_current_board_data (
        int num_samples, float *data_buf, double *ts_buf, int *returned_samples);
    int get_board_data_count (int *result);
    int get_board_data (int data_count, float *data_buf, double *ts_buf);

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
};
