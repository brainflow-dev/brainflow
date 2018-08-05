#include <unistd.h>
#include <string.h>

#include "Board.h"
#include "serial.h"

Board::Board (int num_channels, const char *port_name)
{
    strcpy (this->port_name, port_name);
    this->num_channels = num_channels;

    is_streaming = false;
    keep_alive = false;
    initialized = false;
    logger = spdlog::stderr_logger_mt ("board_logger");
    db = NULL;
    port_descriptor = 0;
}

Board::~Board ()
{
    if (is_streaming)
        this->stop_stream ();
    if (db)
    {
        delete db;
        db = NULL;
    }
    initialized = false;
}

int Board::open_port ()
{
    if (is_port_open (port_descriptor))
        return PORT_ALREADY_OPEN_ERROR;

    int res = open_serial_port (port_name, &port_descriptor);
    if (res < 0)
        return UNABLE_TO_OPEN_PORT_ERROR;

    return STATUS_OK;
}

int Board::send_to_board (char *message)
{
    int res = send_to_serial_port (message, port_descriptor);
    if (res != 1)
        return BOARD_WRITE_ERROR;

    return STATUS_OK;
}

int Board::set_port_settings ()
{
    // only file operations are supported in emulators   
#ifdef EMULATOR_MODE
    return send_to_board ("v");
#endif

    int res = set_serial_port_settings (port_descriptor);
    if (res < 0)
    {
        logger->error ("Unable to set port settings");
        return SER_PORT_ERROR;
    }
    return send_to_board ("v");
}

int Board::status_check ()
{
    unsigned char buf[1];
    int count = 0;

    // board is ready if there are '$$$'
    for (int i = 0; i < 1000; i++)
    {
        int res = read_from_serial_port (port_descriptor, buf, 1);
        if (res > 0)
        {
            if (buf[0] == '$')
                count++;
            else
                count = 0;
            if (count == 3)
                return STATUS_OK;
        }
    }
    return BOARD_NOT_READY_ERROR;
}

int Board::prepare_session ()
{
    if (initialized)
    {
        logger->info ("Session already prepared");
        return STATUS_OK;
    }
    int port_open = open_port ();
    if (port_open != STATUS_OK)
        return port_open;

    int set_settings = set_port_settings ();
    if (set_settings != STATUS_OK)
        return set_settings;

    int initted = status_check ();
    if (initted != STATUS_OK)
        return initted;

    initialized = true;
    return STATUS_OK;
}

int Board::start_stream (int buffer_size)
{
    if (is_streaming)
    {
        logger->error ("Streaming thread already running");
        return STREAM_ALREADY_RUN_ERROR;
    }
    if (buffer_size <= 0 || buffer_size > MAX_CAPTURE_SAMPLES)
    {
        logger->error ("invalid array size");
        return INVALID_BUFFER_SIZE_ERROR;
    }
    
    if (db)
    {
        delete db;
        db = NULL;
    }

    // start streaming
    int send_res = send_to_board ("b");
    if (send_res != STATUS_OK)
        return send_res;

    db = new DataBuffer (num_channels, buffer_size);
    if (!db->is_ready ())
    {
        logger->error ("unable to prepare buffer");
        return INVALID_BUFFER_SIZE_ERROR;
    }

    keep_alive = true;
    streaming_thread = std::thread ([this] {this->read_thread ();});
    is_streaming = true;
    return STATUS_OK;
}

int Board::stop_stream ()
{
    if (is_streaming)
    {
        keep_alive = false;
        is_streaming = false;
        streaming_thread.join ();
        return send_to_board ("s");
    }
    else
        return STREAM_THREAD_IS_NOT_RUNNING;
}

int Board::release_session ()
{
    if (initialized)
    {
        if (is_streaming)
            stop_stream ();

        if (db)
        {
            delete db;
            db = NULL;
        }

        initialized = false;
    }
    return STATUS_OK;
}

int Board::get_current_board_data (int num_samples, float *data_buf, double *ts_buf, int *returned_samples)
{
    if (db && data_buf && ts_buf && returned_samples)
    {
        size_t result = db->get_current_data (num_samples, ts_buf, data_buf);
        (*returned_samples) = int (result);
        return STATUS_OK;
    } else
        return INVALID_ARGUMENTS_ERROR;
}

int Board::get_board_data_count (int *result)
{
    if (!db)
        return EMPTY_BUFFER_ERROR;
    if (!result)
        return INVALID_ARGUMENTS_ERROR;

    *result = int (db->get_data_count ());
    return STATUS_OK;
}

int Board::get_board_data (int data_count, float *data_buf, double *ts_buf)
{
    if (!db)
        return EMPTY_BUFFER_ERROR;
    if ((!data_buf) || !(ts_buf))
        return INVALID_ARGUMENTS_ERROR;

    db->get_data (data_count, ts_buf, data_buf);
    return STATUS_OK;
}
