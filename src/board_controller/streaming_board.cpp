#include <string.h>

#include "board_info_getter.h"
#include "streaming_board.h"

#ifndef _WIN32
#include <errno.h>
#endif


StreamingBoard::StreamingBoard (struct BrainFlowInputParams params)
    : Board ((int)STREAMING_BOARD,
          params) // its a hack - set board_id for streaming board here temporary and override it
                  // with master board id in prepare_session, board_id is protected and there is no
                  // api to get it so its ok
{
    client = NULL;
    is_streaming = false;
    keep_alive = false;
    initialized = false;
}

StreamingBoard::~StreamingBoard ()
{
    skip_logs = true;
    release_session ();
}

int StreamingBoard::prepare_session ()
{
    if (initialized)
    {
        safe_logger (spdlog::level::info, "Session is already prepared");
        return STATUS_OK;
    }
    if ((params.ip_address.empty ()) || (params.other_info.empty ()) || (params.ip_port == 0))
    {
        safe_logger (spdlog::level::err,
            "write multicast group ip to ip_address field, ip port to ip_port field and original "
            "board id to other info");
        return INVALID_ARGUMENTS_ERROR;
    }
    try
    {
        board_id = std::stoi (params.other_info);
    }
    catch (const std::exception &e)
    {
        safe_logger (spdlog::level::err,
            "Write board id for the board which streams data to other_info field");
        safe_logger (spdlog::level::err, e.what ());
        return INVALID_ARGUMENTS_ERROR;
    }

    client = new MultiCastClient (params.ip_address.c_str (), params.ip_port);
    int res = client->init ();
    if (res != (int)MultiCastReturnCodes::STATUS_OK)
    {
#ifdef _WIN32
        safe_logger (spdlog::level::err, "WSAGetLastError is {}", WSAGetLastError ());
#else
        safe_logger (spdlog::level::err, "errno {} message {}", errno, strerror (errno));
#endif
        safe_logger (spdlog::level::err, "failed to init socket: {}", res);
        return GENERAL_ERROR;
    }
    initialized = true;
    return STATUS_OK;
}

int StreamingBoard::config_board (char *config)
{
    // dont allow streaming boards to change config for master board
    return UNSUPPORTED_BOARD_ERROR;
}

int StreamingBoard::start_stream (int buffer_size, char *streamer_params)
{
    if (is_streaming)
    {
        safe_logger (spdlog::level::err, "Streaming thread already running");
        return STREAM_ALREADY_RUN_ERROR;
    }
    if (buffer_size <= 0 || buffer_size > MAX_CAPTURE_SAMPLES)
    {
        safe_logger (spdlog::level::err, "invalid array size");
        return INVALID_BUFFER_SIZE_ERROR;
    }

    if (db)
    {
        delete db;
        db = NULL;
    }
    if (streamer)
    {
        delete streamer;
        streamer = NULL;
    }

    int res = prepare_streamer (streamer_params);
    if (res != STATUS_OK)
    {
        return res;
    }
    int num_channels = 0;
    res = get_num_rows (board_id, &num_channels);
    if (res != STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to get num rows for {}", board_id);
        return res;
    }
    db = new DataBuffer (num_channels - 1, buffer_size); // -1 due to timestamps
    if (!db->is_ready ())
    {
        safe_logger (spdlog::level::err, "unable to prepare buffer");
        return INVALID_BUFFER_SIZE_ERROR;
    }

    keep_alive = true;
    streaming_thread = std::thread ([this] { this->read_thread (); });
    is_streaming = true;
    return STATUS_OK;
}

int StreamingBoard::stop_stream ()
{
    if (is_streaming)
    {
        keep_alive = false;
        is_streaming = false;
        streaming_thread.join ();
        if (streamer)
        {
            delete streamer;
            streamer = NULL;
        }
        return STATUS_OK;
    }
    else
    {
        return STREAM_THREAD_IS_NOT_RUNNING;
    }
}

int StreamingBoard::release_session ()
{
    if (initialized)
    {
        if (is_streaming)
        {
            stop_stream ();
        }
        initialized = false;
        if (client)
        {
            delete client;
            client = NULL;
        }
    }
    return STATUS_OK;
}

void StreamingBoard::read_thread ()
{
    // format for incomming package is determined by original board
    int num_channels = 0;
    get_num_rows (board_id, &num_channels);
    int bytes_per_recv = sizeof (double) * num_channels;
    double *package = new double[num_channels];
    num_channels--;
    int timestamp_channel = 0;
    get_timestamp_channel (board_id, &timestamp_channel);
    int res = 0;

    while (keep_alive)
    {
        res = client->recv (package, bytes_per_recv);
        if (res != bytes_per_recv)
        {
            safe_logger (
                spdlog::level::trace, "unable to read {} bytes, read {}", bytes_per_recv, res);
            continue;
        }

        double timestamp = package[timestamp_channel];
        streamer->stream_data (package, num_channels, timestamp);
        db->add_data (timestamp,
            package); // here package is bigger but add_data will not copy bytes for timestamp
    }
    delete[] package;
}
