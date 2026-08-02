#pragma once

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "board.h"
#include "board_controller.h"
#include "serial.h"

#include "shimmer3_defines.h"

class Shimmer3 : public Board
{
public:
    Shimmer3 (struct BrainFlowInputParams params);
    ~Shimmer3 () override;

    int prepare_session () override;
    int start_stream (int buffer_size, const char *streamer_params) override;
    int stop_stream () override;
    int release_session () override;
    int config_board (std::string config, std::string &response) override;
    int get_board_sampling_rate (int preset) override;

private:
    // Describes one field inside one streamed sample, in transmit order.
    struct PacketField
    {
        shimmer3::Signal signal;
        shimmer3::FieldFormat format;
    };

    std::atomic<bool> keep_alive;
    bool initialized;

    Serial *serial_port;
    std::string port_name;

    std::thread streaming_thread;

    // First-data handshake between start_stream and read_thread.
    std::mutex sync_mutex;
    std::condition_variable sync_cv;
    bool first_data_received;

    double sampling_rate;
    double package_num;
    std::vector<PacketField> packet_layout;
    int packet_data_size;
    uint8_t samples_per_packet;
    uint8_t configured_gsr_range;

    // -- low-level serial helpers --
    int write_bytes (const uint8_t *data, int len);
    int read_exact (uint8_t *buf, int len, bool cancellable = false);
    int read_byte (uint8_t &out, bool cancellable = false);
    int wait_for_ack ();

    // -- device commands --
    int cmd_get_fw_version ();
    int cmd_get_hw_version (uint8_t &hw_version);
    int cmd_disable_instream_ack_prefix ();
    int cmd_set_sensors (uint32_t bitfield);
    int cmd_set_sampling_rate (double hz);
    int cmd_inquiry ();
    int cmd_start_streaming ();
    int cmd_stop_streaming ();

    // -- helpers --
    bool build_packet_layout (const std::vector<shimmer3::Signal> &signals);
    bool convert_gsr_to_microsiemens (int32_t raw, double &eda_us) const;
    void route_field (shimmer3::Signal s, int32_t raw, double *package, int &other_idx);
    void read_thread ();
};
