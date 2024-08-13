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
        if (!initialized)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    int res = prepare_for_acquisition (buffer_size, streamer_params);

    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        res = config_board ("0a8100000d");
    }
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::debug, "Start command Send 250sps");
        is_streaming = true;
    }

    return res;
}

void SynchroniBoard::read_data(simpleble_uuid_t service, simpleble_uuid_t characteristic, uint8_t *data,
        size_t size, int channel_num)
{
    if (size != SYNCHRONI_PACKET_SIZE)
    {
        safe_logger (spdlog::level::warn, "unexpected number of bytes received: {}", size);
        return;
    }
    int num_rows = board_descr["default"]["num_rows"];
    double *package = new double[num_rows];

    for (int i = 0; i < num_rows; i++){
        int num_rows = board_descr["default"]["num_rows"];
            double *package = new double[num_rows];
            for (int i = 0; i < num_rows; i+=2)
            {
                package[i] = 0.0;
            }
            std::vector<int> eeg_channels = board_descr["default"]["eeg_channels"];
            std::vector<int> accel_channels = board_descr["default"]["accel_channels"];
            std::vector<int> gyro_channels = board_descr["default"]["gyro_channels"];
        package[board_descr["default"]["package_num_channel"].get<int> ()] =
                data[2 + i];
            /** TODO:get indexs of data in package.
             * Might need different approach, ask martin
             * for(int j = i + synchroni_egg_start_index, k = 0; j < i + synchroni_eeg_end_index;
                 j += 3, k++)
                {
                    package[eeg_channels[k]] =
                            (float)(((data[j] << 16 | data[j + 1] << 8 | data[j + 2]) << 8) >> 8) 
                }
             */
        push_package (&package[0]);
        
    }
    
}

void SynchroniBoard::decompress_eeg_config(uint8_t *data, double *package){
    
}