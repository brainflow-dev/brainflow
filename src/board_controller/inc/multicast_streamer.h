#pragma once

#include "multicast_server.h"

#include "streamer.h"


class MultiCastStreamer : public Streamer
{

public:
    MultiCastStreamer (const char *ip, int port);
    ~MultiCastStreamer ();

    int init_streamer ();
    void stream_data (double *data, int len, double timestamp);

private:
    char ip[128];
    int port;
    MultiCastServer *server;
};
