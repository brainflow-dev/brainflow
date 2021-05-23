#include <string.h>

#include "board.h"
#include "brainflow_constants.h"
#include "file_streamer.h"
#include "multicast_streamer.h"


MultiCastStreamer::MultiCastStreamer (const char *ip, int port, int data_len) : Streamer (data_len)
{
    strcpy (this->ip, ip);
    this->port = port;
    server = NULL;
    is_streaming = false;
    db = NULL;
}

MultiCastStreamer::~MultiCastStreamer ()
{
    if ((streaming_thread.joinable ()) && (is_streaming))
    {
        is_streaming = false;
        streaming_thread.join ();
    }
    if (server != NULL)
    {
        delete server;
        server = NULL;
    }
    if (db != NULL)
    {
        delete db;
        db = NULL;
    }
}

int MultiCastStreamer::init_streamer ()
{
    if ((is_streaming) || (server != NULL) || (db != NULL))
    {
        Board::board_logger->error ("multicast streamer is running");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }

    server = new MultiCastServer (ip, port);
    int res = server->init ();
    if (res != (int)MultiCastReturnCodes::STATUS_OK)
    {
        delete server;
        server = NULL;
        Board::board_logger->error ("failed to init server multicast socket {}", res);
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }

    db = new DataBuffer (len, 1000);
    if (!db->is_ready ())
    {
        Board::board_logger->error ("unable to prepare buffer for multicast");
        delete db;
        db = NULL;
        delete server;
        server = NULL;
        return (int)BrainFlowExitCodes::INVALID_BUFFER_SIZE_ERROR;
    }

    is_streaming = true;
    streaming_thread = std::thread ([this] { this->thread_worker (); });
    return (int)BrainFlowExitCodes::STATUS_OK;
}

void MultiCastStreamer::stream_data (double *data)
{
    db->add_data (data);
}

void MultiCastStreamer::thread_worker ()
{
    int num_packages = MultiCastStreamer::get_packages_in_chunk ();
    int transaction_len = num_packages * len;
    double *transaction = new double[transaction_len];
    for (int i = 0; i < transaction_len; i++)
    {
        transaction[i] = 0.0;
    }
    while (is_streaming)
    {
        if (db->get_data_count () >= (size_t)num_packages)
        {
            db->get_data (num_packages, transaction);
            server->send (transaction, sizeof (double) * transaction_len);
        }
    }
    delete[] transaction;
}
