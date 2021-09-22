#include "muse_2.h"

Muse2::Muse2 (struct BrainFlowInputParams params)
    : BLELibBoard ((int)BoardIds::MUSE_2_BOARD, params)
{
    keep_alive = false;
    initialized = false;
}

Muse2::~Muse2 ()
{
    skip_logs = true;
    release_session ();
}

int Muse2::prepare_session ()
{
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int Muse2::start_stream (int buffer_size, const char *streamer_params)
{
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int Muse2::stop_stream ()
{
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int Muse2::release_session ()
{
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int Muse2::config_board (std::string config, std::string &response)
{
    return (int)BrainFlowExitCodes::STATUS_OK;
}

void Muse2::read_thread ()
{
}