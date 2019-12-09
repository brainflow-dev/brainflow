#include <string.h>

#include "board.h"
#include "brainflow_constants.h"
#include "file_streamer.h"
#include "multicast_streamer.h"


MultiCastStreamer::MultiCastStreamer (const char *ip, int port) : Streamer ()
{
    strcpy (this->ip, ip);
    this->port = port;
    server = NULL;
}

MultiCastStreamer::~MultiCastStreamer ()
{
    if (server != NULL)
    {
        delete server;
        server = NULL;
    }
}

int MultiCastStreamer::init_streamer ()
{
    server = new MultiCastServer (ip, port);
    int res = server->init ();
    if (res != (int)MultiCastReturnCodes::STATUS_OK)
    {
        Board::board_logger->error ("failed to init server multicast socket {}", res);
        return GENERAL_ERROR;
    }
    return STATUS_OK;
}

void MultiCastStreamer::stream_data (double *data, int len, double timestamp)
{
    double *send_array = new double[len + 1];
    memcpy (send_array, data, sizeof (double) * len);
    send_array[len] = timestamp;
    server->send (send_array, sizeof (double) * (len + 1));
    delete[] send_array;
}
