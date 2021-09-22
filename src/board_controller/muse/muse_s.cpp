#include "muse_s.h"

MuseS::MuseS (struct BrainFlowInputParams params)
    : BLELibBoard ((int)BoardIds::MUSE_S_BOARD, params)
{
    keep_alive = false;
    initialized = false;
}

MuseS::~MuseS ()
{
    skip_logs = true;
    release_session ();
}

int MuseS::prepare_session ()
{
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int MuseS::start_stream (int buffer_size, char *streamer_params)
{
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int MuseS::stop_stream ()
{
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int MuseS::release_session ()
{
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int MuseS::config_board (std::string config, std::string &response)
{
    return (int)BrainFlowExitCodes::STATUS_OK;
}

void MuseS::read_thread ()
{
}