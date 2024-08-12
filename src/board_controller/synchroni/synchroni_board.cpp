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

void SynchroniBoard::read_data(simpleble_uuid_t service, simpleble_uuid_t characteristic, uint8_t *data,
        size_t size, int channel_num)
{
    if (size != 2)
    {
        safe_logger (spdlog::level::warn, "unexpected number of bytes received: {}", size);
        return;
    }
    int num_rows = board_descr["default"]["num_rows"];
    double *package = new double[num_rows];

    for (int i = 0; i < num_rows; i++){
        package[i] = 0.0;
    }
    if (data[0] == 0xA)
    {
        if (data[1] == 0x1)
        {

        }
    }
}

void SynchroniBoard::decompress_eeg_config(uint8_t *data, double *package){
    
}