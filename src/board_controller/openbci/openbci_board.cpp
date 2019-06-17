#include <string.h>

#include "openbci_board.h"
#include "serial.h"

OpenBCIBoard::OpenBCIBoard (int num_channels, const char *port_name) : serial (port_name)
{
    this->num_channels = num_channels;

    is_streaming = false;
    keep_alive = false;
    initialized = false;

    db = NULL;
}

OpenBCIBoard::~OpenBCIBoard ()
{
    release_session ();
}

int OpenBCIBoard::open_port ()
{
    if (serial.is_port_open ())
    {
        Board::board_logger->error ("port {} already open", serial.get_port_name ());
        return PORT_ALREADY_OPEN_ERROR;
    }

    Board::board_logger->info ("openning port {}", serial.get_port_name ());
    int res = serial.open_serial_port ();
    if (res < 0)
    {
        return UNABLE_TO_OPEN_PORT_ERROR;
    }

    return STATUS_OK;
}

int OpenBCIBoard::send_to_board (const char *message)
{
    int res = serial.send_to_serial_port (message);
    if (res != 1)
        return BOARD_WRITE_ERROR;

    return STATUS_OK;
}

int OpenBCIBoard::set_port_settings ()
{
    int res = serial.set_serial_port_settings ();
    if (res < 0)
    {
        Board::board_logger->error ("Unable to set port settings, res is {}", res);
        return SET_PORT_ERROR;
    }
    return send_to_board ("v");
}

int OpenBCIBoard::status_check ()
{
    unsigned char buf[1];
    int count = 0;

    // board is ready if there are '$$$'
    for (int i = 0; i < 500; i++)
    {
        int res = serial.read_from_serial_port (buf, 1);
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

int OpenBCIBoard::prepare_session ()
{
    if (initialized)
    {
        Board::board_logger->info ("Session already prepared");
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

int OpenBCIBoard::start_stream (int buffer_size)
{
    if (is_streaming)
    {
        Board::board_logger->error ("Streaming thread already running");
        return STREAM_ALREADY_RUN_ERROR;
    }
    if (buffer_size <= 0 || buffer_size > MAX_CAPTURE_SAMPLES)
    {
        Board::board_logger->error ("invalid array size");
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
        Board::board_logger->error ("unable to prepare buffer");
        return INVALID_BUFFER_SIZE_ERROR;
    }

    keep_alive = true;
    streaming_thread = std::thread ([this] { this->read_thread (); });
    is_streaming = true;
    return STATUS_OK;
}

int OpenBCIBoard::stop_stream ()
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

int OpenBCIBoard::release_session ()
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
    serial.close_serial_port ();
    return STATUS_OK;
}

int OpenBCIBoard::get_current_board_data (
    int num_samples, float *data_buf, double *ts_buf, int *returned_samples)
{
    if (db && data_buf && ts_buf && returned_samples)
    {
        size_t result = db->get_current_data (num_samples, ts_buf, data_buf);
        (*returned_samples) = int(result);
        return STATUS_OK;
    }
    else
        return INVALID_ARGUMENTS_ERROR;
}

int OpenBCIBoard::get_board_data_count (int *result)
{
    if (!db)
        return EMPTY_BUFFER_ERROR;
    if (!result)
        return INVALID_ARGUMENTS_ERROR;

    *result = int(db->get_data_count ());
    return STATUS_OK;
}

int OpenBCIBoard::get_board_data (int data_count, float *data_buf, double *ts_buf)
{
    if (!db)
        return EMPTY_BUFFER_ERROR;
    if ((!data_buf) || !(ts_buf))
        return INVALID_ARGUMENTS_ERROR;

    db->get_data (data_count, ts_buf, data_buf);
    return STATUS_OK;
}
