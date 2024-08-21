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

    static constexpr int synchroni_packet_size = 220;
    static constexpr int synchroni_single_packet_size = 44;
    static constexpr int num_of_packets = synchroni_packet_size / synchroni_single_packet_size;

    static constexpr int synchroni_packet_index = (synchroni_single_packet_size - 3);

    static constexpr int synchroni_eeg_data_szie = 24;
    static constexpr int synchroni_eeg_Start_index = 4;
    static constexpr int synchroni_eeg_end_index =
        (synchroni_eeg_Start_index + synchroni_eeg_data_szie);

    static constexpr int synchroni_axl_data_size = 6;
    static constexpr int synchroni_gyro_data_size = 6;
    static constexpr int synchroni_axl_start_index = synchroni_eeg_end_index;
    static constexpr int synchroni_axl_end_index =
        synchroni_axl_start_index + synchroni_axl_data_size;
    static constexpr int synchroni_gyro_start_index = synchroni_axl_end_index;
    static constexpr int synchroni_gyro_end_index =
        synchroni_gyro_start_index + synchroni_gyro_data_size;
    static constexpr int FSR_Value = 8388607;
    static constexpr int sc_brainflow_package_size = 17;

    static constexpr int synchroni_handshaking_packet_size = 6;
protected:
    volatile simpleble_adapter_t synchroni_adapter;
    volatile simpleble_peripheral_t synchroni_peripheral;
    bool initialized;
    bool is_streaming;
    std::mutex m;
    std::condition_variable cv;
    std::pair<simpleble_uuid_t, simpleble_uuid_t> notified_characteristics;
    std::pair<simpleble_uuid_t, simpleble_uuid_t> write_characteristics;
};