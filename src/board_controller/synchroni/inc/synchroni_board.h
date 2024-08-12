#pragma once


#include "board.h"
#include "board_controller.h"
#include "ble_lib_board.h"

class SynchroniBoard : public BLELibBoard
{

private:


public:
    SynchroniBoard (struct BrainFlowInputParams params);
    ~SynchroniBoard ();

    int prepare_session ();
    int start_stream (int buffer_size, const char *streamer_params);
    int stop_stream ();
    int release_session ();
    int config_board (std::string config, std::string &response);
    int config_board (std::string config);
    void adapter_1_on_scan_found (simpleble_adapter_t adapter, simpleble_peripheral_t peripheral);
    void read_data (simpleble_uuid_t service, simpleble_uuid_t characteristic, uint8_t *data,
        size_t size, int channel_num);
protected:
    void SynchroniBoard::decompress_eeg_config(uint8_t *data, double *package);
    
};