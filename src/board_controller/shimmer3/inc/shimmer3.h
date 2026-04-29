/*
 * Shimmer3 board driver for BrainFlow.
 *
 * Adapted from the pyshimmer Python library by semoo-lab:
 *   https://github.com/seemoo-lab/pyshimmer
 *
 * Original work licensed under the GNU General Public License v3.0.
 * See https://www.gnu.org/licenses/gpl-3.0.html for details.
 */

#pragma once

#include <atomic>
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

private:
    volatile bool keep_alive;
    volatile bool initialized;

    Serial *serial_port;
    std::string port_name;

    std::thread streaming_thread;
    std::mutex m;

    // Populated during prepare_session by querying the device
    double sampling_rate;
    std::vector<EChannelType> active_channels;
    std::vector<ShimmerChannelDType> active_dtypes;
    int packet_size;
    ShimmerAllCalibration calibration;

    // Serial helpers
    int serial_write (const uint8_t *data, int len);
    int serial_read (uint8_t *buf, int len);
    int serial_read_byte (uint8_t &out);
    int send_command (uint8_t cmd);
    int send_command_with_args (uint8_t cmd, const uint8_t *args, int args_len);
    int wait_for_ack ();
    int read_response (uint8_t expected_code, uint8_t *buf, int buf_len, int &out_len);

    // Device commands
    int get_firmware_version (uint16_t &fw_type, uint16_t &major, uint8_t &minor, uint8_t &rel);
    int get_sampling_rate (double &sr);
    int set_sampling_rate (double sr);
    int get_shimmer_version (uint8_t &hw_ver);
    int send_inquiry (double &sr, int &buf_size, std::vector<EChannelType> &channels);
    int set_sensors (uint32_t sensor_bitfield);
    int start_streaming_cmd ();
    int stop_streaming_cmd ();
    int disable_status_ack ();
    int get_all_calibration (ShimmerAllCalibration &cal);

    // Packet parsing
    bool lookup_channel_dtype (EChannelType ch, ShimmerChannelDType &out);
    void build_active_channel_list (const std::vector<EChannelType> &inquiry_channels);
    int compute_packet_size ();

    void read_thread ();

public:
    Shimmer3 (struct BrainFlowInputParams params);
    ~Shimmer3 ();

    int prepare_session ();
    int start_stream (int buffer_size, const char *streamer_params);
    int stop_stream ();
    int release_session ();
    int config_board (std::string config, std::string &response);
};
