#pragma once

#include <condition_variable>
#include <mutex>
#include <utility>
#include <vector>

#include "ble_lib_board.h"


class MuseAnthena : public BLELibBoard
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

    std::string bytes_to_string (const uint8_t *data, size_t size);

public:
    MuseAnthena (int board_id, struct BrainFlowInputParams params);
    ~MuseAnthena ();

    int prepare_session ();
    int start_stream (int buffer_size, const char *streamer_params);
    int stop_stream ();
    int release_session ();
    int config_board (std::string config, std::string &response);
    int config_board (std::string config);

    void adapter_on_scan_found (simpleble_adapter_t adapter, simpleble_peripheral_t peripheral);
    void peripheral_on_eeg (simpleble_peripheral_t peripheral, simpleble_uuid_t service,
        simpleble_uuid_t characteristic, const uint8_t *data, size_t size);
    void peripheral_on_aux (simpleble_peripheral_t peripheral, simpleble_uuid_t service,
        simpleble_uuid_t characteristic, const uint8_t *data, size_t size);
    void peripheral_on_status (simpleble_peripheral_t peripheral, simpleble_uuid_t service,
        simpleble_uuid_t characteristic, const uint8_t *data, size_t size);
};
