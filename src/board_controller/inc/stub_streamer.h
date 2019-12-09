#pragma once

#include "brainflow_constants.h"
#include "streamer.h"


class StubStreamer : public Streamer
{

public:
    StubStreamer ()
    {
    }
    ~StubStreamer ()
    {
    }

    int init_streamer ()
    {
        return STATUS_OK;
    }
    void stream_data (double *data, int size, double timestamp)
    {
    }
};
