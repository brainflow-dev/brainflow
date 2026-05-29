#pragma once

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
    ~Shimmer3 ();

    int prepare_session () override;
    int start_stream (int buffer_size, const char *streamer_params) override;
    int stop_stream () override;
    int release_session () override;
    int config_board (std::string config, std::string &response) override;

private:
    // Describes one field inside the streamed data packet, in transmit order.
    struct PacketField
    {
        shimmer3::Signal signal;
        shimmer3::FieldFormat format;
    };

    volatile bool keep_alive;
    volatile bool initialized;

    Serial *serial_port;
    std::string port_name;

    std::thread streaming_thread;

    // First-data handshake: start_stream blocks until
    // read_thread confirms real data packets are flowing, or times out.
    std::mutex sync_mutex;
    std::condition_variable sync_cv;
    bool first_data_received;

    double sampling_rate;
    double package_num;                     // local sequence counter (Shimmer3
                                            // packets carry no sequence number)
    std::vector<PacketField> packet_layout; // leading timestamp + active signals
    int packet_data_size;                   // bytes after the 0x00 header

    // -- low-level serial helpers --
    int write_bytes (const uint8_t *data, int len);
    int read_exact (uint8_t *buf, int len);
    int read_byte (uint8_t &out);
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
    void build_packet_layout (const std::vector<shimmer3::Signal> &signals);
    void read_thread ();
    int route_field (shimmer3::Signal s, int32_t raw, double *package, int &accel_axis,
        int &gyro_axis, int &mag_axis, int &exg_idx, int &other_idx);
};
