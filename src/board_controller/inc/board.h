#ifndef BOARD
#define BOARD

#include "data_buffer.h"
#include "spdlog/spdlog.h"

class Board
{
public:
    static std::shared_ptr<spdlog::logger> board_logger;
    static int set_log_level (int level);

    virtual ~Board ()
    {
        if (db != NULL)
        {
            delete db;
            db = NULL;
        }
    }
    Board ()
    {
        // should be initialized in start_stream
        db = NULL;
    }
    virtual int prepare_session () = 0;
    virtual int start_stream (int buffer_size) = 0;
    virtual int stop_stream () = 0;
    virtual int release_session () = 0;
    virtual int get_board_id () = 0;

    int get_current_board_data (
        int num_samples, float *data_buf, double *ts_buf, int *returned_samples);
    int get_board_data_count (int *result);
    int get_board_data (int data_count, float *data_buf, double *ts_buf);

protected:
    DataBuffer *db;
};

#endif
