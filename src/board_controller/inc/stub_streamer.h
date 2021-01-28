#pragma once

#include "brainflow_constants.h"
#include "streamer.h"


class StubStreamer : public Streamer
{

public:
    StubStreamer (int data_len) : Streamer (data_len)
    {
    }
    ~StubStreamer ()
    {
    }

    int init_streamer ()
    {
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    void stream_data (double *data)
    {
    }
};
