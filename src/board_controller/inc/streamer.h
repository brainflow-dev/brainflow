#pragma once


class Streamer
{
public:
    Streamer (int len)
    {
        this->len = len;
    }
    virtual ~Streamer ()
    {
    }

    virtual int init_streamer () = 0;
    virtual void stream_data (double *data) = 0;

protected:
    int len;
};
