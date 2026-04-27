#pragma once

#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include "ble_lib_board.h"
#include "muse_anthena_types.h"


class MuseAnthena : public BLELibBoard
{

protected:
    static const size_t PACKET_HEADER_SIZE = 14;
    static const size_t SUBPACKET_HEADER_SIZE = 5;

    struct SensorConfig
    {
        SensorType type;
        int n_channels;
        int n_samples;
        double sampling_rate;
        size_t data_len;
        bool variable_length;

        SensorConfig ();
        SensorConfig (SensorType type, int n_channels, int n_samples, double sampling_rate,
            size_t data_len, bool variable_length = false);
    };

    volatile simpleble_adapter_t muse_adapter;
    volatile simpleble_peripheral_t muse_peripheral;
    bool initialized;
    bool is_streaming;
    std::mutex m;
    std::mutex callback_lock;
    std::condition_variable cv;
    std::vector<std::pair<simpleble_uuid_t, simpleble_uuid_t>> notified_characteristics;
    std::pair<simpleble_uuid_t, simpleble_uuid_t> control_characteristics;
    double last_eeg_timestamp;
    double last_aux_timestamp;
    double last_anc_timestamp;
    double last_battery;
    std::string muse_preset;
    bool enable_low_latency;

    static std::string trim_string (const std::string &value);
    static std::string to_lower (const std::string &value);
    static bool is_valid_muse_preset (const std::string &preset);
    static bool parse_bool_option (const std::string &value, bool &parsed);
    int parse_muse_options ();
    std::string bytes_to_string (const uint8_t *data, size_t size);
    void handle_data_notification (const uint8_t *data, size_t size);
    void parse_sensor_payload (
        uint8_t tag, uint8_t sequence_num, double host_timestamp, const uint8_t *data, size_t size);
    bool get_sensor_config (uint8_t tag, SensorConfig &config);
    int get_optics_canonical_index (uint8_t tag, int channel);
    void reset_timestamps ();
    static double get_sample_timestamp (double last_timestamp, double current_timestamp,
        int sample_index, int n_samples, double sampling_rate);

public:
    MuseAnthena (int board_id, struct BrainFlowInputParams params);
    ~MuseAnthena () override;

    int prepare_session () override;
    int start_stream (int buffer_size, const char *streamer_params) override;
    int stop_stream () override;
    int release_session () override;
    int config_board (std::string config, std::string &response) override;
    int config_board (std::string config);

    void adapter_on_scan_found (simpleble_adapter_t adapter, simpleble_peripheral_t peripheral);
    void peripheral_on_data (simpleble_peripheral_t peripheral, simpleble_uuid_t service,
        simpleble_uuid_t characteristic, const uint8_t *data, size_t size);
    void peripheral_on_status (simpleble_peripheral_t peripheral, simpleble_uuid_t service,
        simpleble_uuid_t characteristic, const uint8_t *data, size_t size);
};
