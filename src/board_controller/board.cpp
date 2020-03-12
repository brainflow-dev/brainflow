#include <string>

#include "board.h"
#include "board_controller.h"
#include "file_streamer.h"
#include "multicast_streamer.h"
#include "stub_streamer.h"

#include "spdlog/sinks/null_sink.h"


#define LOGGER_NAME "brainflow_logger"

std::shared_ptr<spdlog::logger> Board::board_logger = spdlog::stderr_logger_mt (LOGGER_NAME);

int Board::set_log_level (int level)
{
    int log_level = level;
    if (level > 6)
    {
        log_level = 6;
    }
    if (level < 0)
    {
        log_level = 0;
    }
    Board::board_logger->set_level (spdlog::level::level_enum (log_level));
    Board::board_logger->flush_on (spdlog::level::level_enum (log_level));
    return STATUS_OK;
}

int Board::set_log_file (char *log_file)
{
    spdlog::level::level_enum level = Board::board_logger->level ();
    Board::board_logger = spdlog::create<spdlog::sinks::null_sink_st> (
        "null_logger"); // to dont set logger to nullptr and avoid raice condition
    spdlog::drop (LOGGER_NAME);
    Board::board_logger = spdlog::basic_logger_mt (LOGGER_NAME, log_file);
    Board::board_logger->set_level (level);
    Board::board_logger->flush_on (level);
    spdlog::drop ("null_logger");
    return STATUS_OK;
}

int Board::prepare_streamer (char *streamer_params)
{
    // to dont write smth like if (streamer) every time for all boards create dummy streamer which
    // does nothing and return an instance of this streamer if user dont specify streamer_params
    if (streamer_params == NULL)
    {
        safe_logger (spdlog::level::debug, "use stub streamer");
        streamer = new StubStreamer ();
    }
    else if (streamer_params[0] == '\0')
    {
        safe_logger (spdlog::level::debug, "use stub streamer");
        streamer = new StubStreamer ();
    }
    else
    {
        // parse string, sscanf doesnt work
        std::string streamer_params_str (streamer_params);
        size_t idx1 = streamer_params_str.find ("://");
        if (idx1 == std::string::npos)
        {
            safe_logger (
                spdlog::level::err, "format is streamer_type://streamer_dest:streamer_args");
            return INVALID_ARGUMENTS_ERROR;
        }
        std::string streamer_type = streamer_params_str.substr (0, idx1);
        size_t idx2 = streamer_params_str.find (":", idx1 + 3);
        if (idx2 == std::string::npos)
        {
            safe_logger (
                spdlog::level::err, "format is streamer_type://streamer_dest:streamer_args");
            return INVALID_ARGUMENTS_ERROR;
        }
        std::string streamer_dest = streamer_params_str.substr (idx1 + 3, idx2 - idx1 - 3);
        std::string streamer_mods = streamer_params_str.substr (idx2 + 1);

        if (streamer_type == "file")
        {
            streamer = new FileStreamer (streamer_dest.c_str (), streamer_mods.c_str ());
        }
        if (streamer_type == "streaming_board")
        {
            int port = 0;
            try
            {
                port = std::stoi (streamer_mods);
            }
            catch (const std::exception &e)
            {
                safe_logger (spdlog::level::err, e.what ());
                return INVALID_ARGUMENTS_ERROR;
            }
            streamer = new MultiCastStreamer (streamer_dest.c_str (), port);
        }

        if (streamer == NULL)
        {
            safe_logger (
                spdlog::level::err, "unsupported streamer type {}", streamer_type.c_str ());
            return INVALID_ARGUMENTS_ERROR;
        }
    }

    int res = streamer->init_streamer ();
    if (res != STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to init streamer");
        delete streamer;
        streamer = NULL;
    }

    return res;
}

int Board::get_current_board_data (int num_samples, double *data_buf, int *returned_samples)
{
    if (db && data_buf && returned_samples)
    {
        int num_data_channels = -1;
        int res = get_num_rows (board_id, &num_data_channels);
        if (res != STATUS_OK)
        {
            return res;
        }
        num_data_channels--; // columns_size includes timestamp channel, which is a separated field
                             // in DataBuffer class

        double *buf = new double[num_samples * num_data_channels];
        double *ts_buf = new double[num_samples];
        int num_data_points = (int)db->get_current_data (num_samples, ts_buf, buf);
        reshape_data (num_data_points, buf, ts_buf, data_buf);
        delete[] buf;
        delete[] ts_buf;
        *returned_samples = num_data_points;
        return STATUS_OK;
    }
    else
    {
        return INVALID_ARGUMENTS_ERROR;
    }
}

int Board::get_board_data_count (int *result)
{
    if (!db)
    {
        return EMPTY_BUFFER_ERROR;
    }
    if (!result)
    {
        return INVALID_ARGUMENTS_ERROR;
    }

    *result = (int)db->get_data_count ();
    return STATUS_OK;
}

int Board::get_board_data (int data_count, double *data_buf)
{
    if (!db)
    {
        return EMPTY_BUFFER_ERROR;
    }
    if (!data_buf)
    {
        return INVALID_ARGUMENTS_ERROR;
    }
    int num_data_channels = 0;
    int res = get_num_rows (board_id, &num_data_channels);
    if (res != STATUS_OK)
    {
        return res;
    }
    num_data_channels--; // columns_size includes timestamp channel, which is a separated field
                         // in DataBuffer class

    double *buf = new double[data_count * num_data_channels];
    double *ts_buf = new double[data_count];
    int num_data_points = (int)db->get_data (data_count, ts_buf, buf);
    reshape_data (num_data_points, buf, ts_buf, data_buf);
    delete[] buf;
    delete[] ts_buf;
    return STATUS_OK;
}

void Board::reshape_data (
    int data_count, const double *buf, const double *ts_buf, double *output_buf)
{
    int num_data_channels = 0;
    get_num_rows (board_id, &num_data_channels); // here we know that board id is valid
    num_data_channels--;                         // -1 because of timestamp

    for (int i = 0; i < data_count; i++)
    {
        for (int j = 0; j < num_data_channels; j++)
        {
            output_buf[j * data_count + i] = buf[i * num_data_channels + j];
        }
    }
    // add timestamp to resulting data table
    for (int i = 0; i < data_count; i++)
    {
        output_buf[num_data_channels * data_count + i] = ts_buf[i];
    }
}
