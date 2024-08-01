#include "synchroni_board.h"
#include <stdint.h>
#include <string.h>
#include "timestamp.h"
#include <string.h>

SynchroniBoard::SynchroniBoard (struct BrainFlowInputParams params)
: BLELibBoard ((int)BoardIds::SYNCHRONI_TRIO,params)
{

}

SynchroniBoard::~SynchroniBoard()
{

}

int SynchroniBoard::prepare_session()
{

}

int SynchroniBoard::start_stream(int buffer_size, const char * streamer_params)
{

}