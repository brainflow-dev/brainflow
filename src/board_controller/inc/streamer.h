#pragma once


class Streamer
{
public:
    Streamer ()
    {
    }
    virtual ~Streamer ()
    {
    }

    virtual int init_streamer () = 0;
    virtual void stream_data (double *data, int len, double timestamp) = 0;
};
