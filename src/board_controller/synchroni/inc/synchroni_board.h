#pragma once


#include "board.h"
#include "board_controller.h"
#include "ble_lib_board.h"
#define CMD_GET_EEG_CONFIG 0xA0
#define CMD_SET_EEG_CONFIG 0xA1
#define CMD_GET_ECG_CONFIG 0xA2
#define CMD_SET_ECG_CONFIG 0xA3
#define CMD_GET_IMPEDANCE_CONFIG 0xA4
#define CMD_SET_IMPEDANCE_CONFIG 0xA5
#define CMD_GET_EEG_CAP 0xA6
#define CMD_GET_ECG_CAP 0xA7
#define CMD_GET_IMPEDANCE_CAP 0xA8
#define CMD_GET_IMU_CAP 0xAB
#define CMD_GET_IMU_CONFIG 0xAC
#define CMD_SET_IMU_CONFIG 0xAD
#define SYNCHRONI_PACKET_SIZE 10
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
    bool initialized;
    bool is_streaming;
};