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
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

void MultiCastStreamer::stream_data (double *data)
{
    server->send (data, sizeof (double) * len);
}
