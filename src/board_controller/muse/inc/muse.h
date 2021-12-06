#pragma once

#include "ble_lib_board.h"
#include <condition_variable>
#include <mutex>
#include <utility>
#include <vector>


class Muse : public BLELibBoard
{

protected:
    volatile simpleble_adapter_t muse_adapter;
    volatile simpleble_peripheral_t muse_peripheral;
    bool initialized;
    bool is_streaming;
    std::mutex m;
    std::condition_variable cv;
    std::vector<std::pair<simpleble_uuid_t, simpleble_uuid_t>> notified_characteristics;
    std::pair<simpleble_uuid_t, simpleble_uuid_t> control_characteristics;
    std::vector<std::vector<double>> current_buf;
    std::vector<bool> new_eeg_data;
    double last_timestamp;
    int current_accel_pos;
    int current_gyro_pos;
    int current_ppg_pos;

    std::mutex callback_lock;

public:
    Muse (int board_id, struct BrainFlowInputParams params);
    ~Muse ();

    int prepare_session ();
    int start_stream (int buffer_size, const char *streamer_params);
    int stop_stream ();
    int release_session ();
    int config_board (std::string config, std::string &response);
    int config_board (std::string config);

    void adapter_on_scan_found (simpleble_adapter_t adapter, simpleble_peripheral_t peripheral);
    void peripheral_on_eeg (simpleble_uuid_t service, simpleble_uuid_t characteristic,
        uint8_t *data, size_t size, size_t channel_num);
    void peripheral_on_ppg (simpleble_uuid_t service, simpleble_uuid_t characteristic,
        uint8_t *data, size_t size, size_t ppg_num);
    void peripheral_on_accel (
        simpleble_uuid_t service, simpleble_uuid_t characteristic, uint8_t *data, size_t size);
    void peripheral_on_gyro (
        simpleble_uuid_t service, simpleble_uuid_t characteristic, uint8_t *data, size_t size);
};
