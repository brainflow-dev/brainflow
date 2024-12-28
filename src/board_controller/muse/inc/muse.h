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
    std::mutex callback_lock;
    std::condition_variable cv;
    std::vector<std::pair<simpleble_uuid_t, simpleble_uuid_t>> notified_characteristics;
    std::pair<simpleble_uuid_t, simpleble_uuid_t> control_characteristics;
    std::vector<std::vector<double>> current_default_buf;
    std::vector<std::vector<double>> current_aux_buf;
    std::vector<std::vector<double>> current_anc_buf;
    std::vector<bool> new_eeg_data;
    std::vector<bool> new_ppg_data;
    double last_fifth_chan_timestamp; // used to determine 4 or 5 channels used
    double last_ppg_timestamp;        // used for timestamp correction
    double last_eeg_timestamp;        // used for timestamp correction
    double last_aux_timestamp;        // used for timestamp correction

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
    void peripheral_on_eeg (simpleble_peripheral_t peripheral, simpleble_uuid_t service,
        simpleble_uuid_t characteristic, const uint8_t *data, size_t size, size_t channel_num);
    void peripheral_on_ppg (simpleble_peripheral_t peripheral, simpleble_uuid_t service,
        simpleble_uuid_t characteristic, const uint8_t *data, size_t size, size_t ppg_num);
    void peripheral_on_accel (simpleble_peripheral_t peripheral, simpleble_uuid_t service,
        simpleble_uuid_t characteristic, const uint8_t *data, size_t size);
    void peripheral_on_gyro (simpleble_peripheral_t peripheral, simpleble_uuid_t service,
        simpleble_uuid_t characteristic, const uint8_t *data, size_t size);
    void peripheral_on_status (simpleble_peripheral_t peripheral, simpleble_uuid_t service,
        simpleble_uuid_t characteristic, const uint8_t *data, size_t size);
};
