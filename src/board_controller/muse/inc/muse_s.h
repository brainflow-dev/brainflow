#pragma once

#include "ble_lib_board.h"
#include <condition_variable>
#include <mutex>
#include <utility>
#include <vector>


class MuseS : public BLELibBoard
{

private:
    volatile simpleble_adapter_t muse_adapter;
    volatile simpleble_peripheral_t muse_peripheral;
    bool initialized;
    std::mutex m;
    std::condition_variable cv;
    std::vector<std::pair<simpleble_uuid_t, simpleble_uuid_t>> notified_characteristics;
    std::pair<simpleble_uuid_t, simpleble_uuid_t> control_characteristics;

public:
    MuseS (struct BrainFlowInputParams params);
    ~MuseS ();

    int prepare_session ();
    int start_stream (int buffer_size, const char *streamer_params);
    int stop_stream ();
    int release_session ();
    int config_board (std::string config, std::string &response);
    int config_board (std::string config);

    void adapter_on_scan_found (simpleble_adapter_t adapter, simpleble_peripheral_t peripheral);
    void peripheral_on_tp9 (
        simpleble_uuid_t service, simpleble_uuid_t characteristic, uint8_t *data, size_t size);
    void peripheral_on_af7 (
        simpleble_uuid_t service, simpleble_uuid_t characteristic, uint8_t *data, size_t size);
    void peripheral_on_af8 (
        simpleble_uuid_t service, simpleble_uuid_t characteristic, uint8_t *data, size_t size);
    void peripheral_on_tp10 (
        simpleble_uuid_t service, simpleble_uuid_t characteristic, uint8_t *data, size_t size);
    void peripheral_on_accel (
        simpleble_uuid_t service, simpleble_uuid_t characteristic, uint8_t *data, size_t size);
    void peripheral_on_gyro (
        simpleble_uuid_t service, simpleble_uuid_t characteristic, uint8_t *data, size_t size);
};
