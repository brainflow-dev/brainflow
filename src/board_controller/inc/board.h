#ifndef BOARD
#define BOARD

#include "spdlog/spdlog.h"

class Board
{
public:
    static std::shared_ptr<spdlog::logger> board_logger;
    static int set_log_level (int level);

    virtual int prepare_session () = 0;
    virtual int start_stream (int buffer_size) = 0;
    virtual int stop_stream () = 0;
    virtual int release_session () = 0;
    virtual int get_current_board_data (
        int num_samples, float *data_buf, double *ts_buf, int *returned_samples) = 0;
    virtual int get_board_data_count (int *result) = 0;
    virtual int get_board_data (int data_count, float *data_buf, double *ts_buf) = 0;
};

#endif
