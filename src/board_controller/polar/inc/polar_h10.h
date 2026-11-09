#pragma once

#include <condition_variable>
#include <mutex>
#include <vector>

#include "ble_lib_board.h"

class PolarH10 : public BLELibBoard
{
    simpleble_adapter_t adapter;
    simpleble_peripheral_t peripheral;
    std::vector<simpleble_peripheral_t> discovered;
    bool control_subscribed;
    bool data_subscribed;
    bool hr_subscribed;
    bool initialized;
    bool streaming;
    bool accept_data;
    bool ready;
    bool awaiting;
    uint8_t operation;
    int control_result;
    std::vector<uint8_t> control_parameters;
    uint32_t ecg_counter;
    uint32_t hr_counter;
    uint64_t last_timestamp;
    double battery;
    std::mutex mutex;
    std::condition_variable received;
    int transact (const std::vector<uint8_t> &command, std::vector<uint8_t> &response);

protected:
    // Transport seam: protocol/lifecycle tests override only these BLE operations.
    virtual int connect_device ();
    virtual bool write_control (const std::vector<uint8_t> &command);
    virtual void disconnect_device ();
    void on_control (const uint8_t *data, size_t size);
    void on_ecg (const uint8_t *data, size_t size);
    void on_heart_rate (const uint8_t *data, size_t size);

public:
    explicit PolarH10 (struct BrainFlowInputParams params);
    ~PolarH10 () override;
    int prepare_session () override;
    int start_stream (int buffer_size, const char *streamer_params) override;
    int stop_stream () override;
    int release_session () override;
    int config_board (std::string config, std::string &response) override;
};
