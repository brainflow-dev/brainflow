/*
 * Shimmer3 board driver for BrainFlow.
 *
 * Adapted from the pyshimmer Python library by semoo-lab:
 *   https://github.com/seemoo-lab/pyshimmer
 *
 * Original work licensed under the GNU General Public License v3.0.
 * See https://www.gnu.org/licenses/gpl-3.0.html for details.
 */

#include <chrono>
#include <cstring>
#include <string>

#include "shimmer3.h"
#include "shimmer3_defines.h"

#include "board_controller.h"
#include "brainflow_constants.h"
#include "custom_cast.h"
#include "get_dll_dir.h"
#include "timestamp.h"


Shimmer3::Shimmer3 (struct BrainFlowInputParams params)
    : Board ((int)BoardIds::SHIMMER3_BOARD, params)
{
    keep_alive = false;
    initialized = false;
    serial_port = NULL;
    sampling_rate = 0.0;
    packet_size = 0;
}

Shimmer3::~Shimmer3 ()
{
    skip_logs = true;
    release_session ();
}


// ---------------------------------------------------------------------------
// Serial helpers
// ---------------------------------------------------------------------------

int Shimmer3::serial_write (const uint8_t *data, int len)
{
    if (serial_port == NULL)
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;

    int res = serial_port->send_to_serial_port (reinterpret_cast<const void *> (data), len);
    if (res != len)
    {
        safe_logger (spdlog::level::err, "Failed to write {} bytes to serial", len);
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int Shimmer3::serial_read (uint8_t *buf, int len)
{
    if (serial_port == NULL)
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;

    int total_read = 0;
    int max_attempts = len * 10;
    while (total_read < len && max_attempts-- > 0)
    {
        int r = serial_port->read_from_serial_port (
            reinterpret_cast<char *> (buf + total_read), len - total_read);
        if (r > 0)
        {
            total_read += r;
        }
        else if (r == 0)
        {
            std::this_thread::sleep_for (std::chrono::milliseconds (1));
        }
        else
        {
            safe_logger (spdlog::level::err, "Serial read error");
            return (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
    }
    if (total_read < len)
    {
        safe_logger (spdlog::level::err, "Serial read timeout: got {}/{}", total_read, len);
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int Shimmer3::serial_read_byte (uint8_t &out)
{
    return serial_read (&out, 1);
}

int Shimmer3::send_command (uint8_t cmd)
{
    return serial_write (&cmd, 1);
}

int Shimmer3::send_command_with_args (uint8_t cmd, const uint8_t *args, int args_len)
{
    int res = send_command (cmd);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
        return res;
    if (args != NULL && args_len > 0)
        return serial_write (args, args_len);
    return (int)BrainFlowExitCodes::STATUS_OK;
}

// Keep reading bytes until we see the ACK byte. The device sometimes
// sends stale status responses before the ACK, so we skip those.
int Shimmer3::wait_for_ack ()
{
    uint8_t byte = 0;
    int max_tries = 256;
    while (max_tries-- > 0)
    {
        int res = serial_read_byte (byte);
        if (res != (int)BrainFlowExitCodes::STATUS_OK)
            return res;
        if (byte == ShimmerBT::ACK_COMMAND_PROCESSED)
            return (int)BrainFlowExitCodes::STATUS_OK;
        if (byte == ShimmerBT::INSTREAM_CMD_RESPONSE)
        {
            uint8_t discard;
            serial_read_byte (discard);
        }
    }
    safe_logger (spdlog::level::err, "Timed out waiting for ACK");
    return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
}

int Shimmer3::read_response (uint8_t expected_code, uint8_t *buf, int buf_len, int &out_len)
{
    uint8_t code = 0;
    int res = serial_read_byte (code);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
        return res;
    if (code != expected_code)
    {
        safe_logger (
            spdlog::level::err, "Expected response 0x{:02X}, got 0x{:02X}", expected_code, code);
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    if (buf != NULL && buf_len > 0)
    {
        res = serial_read (buf, buf_len);
        if (res != (int)BrainFlowExitCodes::STATUS_OK)
            return res;
        out_len = buf_len;
    }
    else
    {
        out_len = 0;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}


// ---------------------------------------------------------------------------
// Device commands
// ---------------------------------------------------------------------------

int Shimmer3::get_firmware_version (
    uint16_t &fw_type, uint16_t &major, uint8_t &minor, uint8_t &rel)
{
    int res = send_command (ShimmerBT::GET_FW_VERSION_COMMAND);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
        return res;
    res = wait_for_ack ();
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
        return res;

    uint8_t buf[6];
    int out_len = 0;
    res = read_response (ShimmerBT::FW_VERSION_RESPONSE, buf, 6, out_len);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
        return res;

    fw_type = static_cast<uint16_t> (buf[0] | (buf[1] << 8));
    major = static_cast<uint16_t> (buf[2] | (buf[3] << 8));
    minor = buf[4];
    rel = buf[5];

    safe_logger (
        spdlog::level::info, "Shimmer FW: type={}, version={}.{}.{}", fw_type, major, minor, rel);
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int Shimmer3::get_shimmer_version (uint8_t &hw_ver)
{
    int res = send_command (ShimmerBT::GET_SHIMMER_VERSION_COMMAND);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
        return res;
    res = wait_for_ack ();
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
        return res;

    uint8_t buf[1];
    int out_len = 0;
    res = read_response (ShimmerBT::SHIMMER_VERSION_RESPONSE, buf, 1, out_len);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
        return res;

    hw_ver = buf[0];
    safe_logger (spdlog::level::info, "Shimmer HW version: {}", hw_ver);
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int Shimmer3::get_sampling_rate (double &sr)
{
    int res = send_command (ShimmerBT::GET_SAMPLING_RATE_COMMAND);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
        return res;
    res = wait_for_ack ();
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
        return res;

    uint8_t buf[2];
    int out_len = 0;
    res = read_response (ShimmerBT::SAMPLING_RATE_RESPONSE, buf, 2, out_len);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
        return res;

    uint16_t dr = static_cast<uint16_t> (buf[0] | (buf[1] << 8));
    sr = Shimmer3Const::dr2sr (dr);
    safe_logger (spdlog::level::info, "Shimmer sampling rate: {} Hz", sr);
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int Shimmer3::set_sampling_rate (double sr)
{
    uint16_t dr = Shimmer3Const::sr2dr (sr);
    uint8_t args[2] = {static_cast<uint8_t> (dr & 0xFF), static_cast<uint8_t> ((dr >> 8) & 0xFF)};
    int res = send_command_with_args (ShimmerBT::SET_SAMPLING_RATE_COMMAND, args, 2);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
        return res;
    return wait_for_ack ();
}

int Shimmer3::set_sensors (uint32_t sensor_bitfield)
{
    uint8_t args[3] = {static_cast<uint8_t> (sensor_bitfield & 0xFF),
        static_cast<uint8_t> ((sensor_bitfield >> 8) & 0xFF),
        static_cast<uint8_t> ((sensor_bitfield >> 16) & 0xFF)};
    int res = send_command_with_args (ShimmerBT::SET_SENSORS_COMMAND, args, 3);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
        return res;
    return wait_for_ack ();
}

int Shimmer3::send_inquiry (double &sr, int &buf_size, std::vector<EChannelType> &channels)
{
    int res = send_command (ShimmerBT::INQUIRY_COMMAND);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
        return res;
    res = wait_for_ack ();
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
        return res;

    uint8_t hdr_code = 0;
    res = serial_read_byte (hdr_code);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
        return res;
    if (hdr_code != ShimmerBT::INQUIRY_RESPONSE)
    {
        safe_logger (spdlog::level::err, "Expected INQUIRY_RESPONSE 0x{:02X}, got 0x{:02X}",
            ShimmerBT::INQUIRY_RESPONSE, hdr_code);
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }

    // Inquiry header: sampling rate (2) + sensor bitfield (4) + n_ch (1) + buf_size (1)
    uint8_t hdr_buf[8];
    res = serial_read (hdr_buf, 8);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
        return res;

    uint16_t sr_val = static_cast<uint16_t> (hdr_buf[0] | (hdr_buf[1] << 8));
    uint8_t n_ch = hdr_buf[6];
    buf_size = hdr_buf[7];
    sr = Shimmer3Const::dr2sr (sr_val);

    // Each following byte identifies one active channel
    std::vector<uint8_t> ch_bytes (n_ch);
    res = serial_read (ch_bytes.data (), n_ch);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
        return res;

    channels.clear ();
    for (int i = 0; i < n_ch; i++)
        channels.push_back (static_cast<EChannelType> (ch_bytes[i]));

    safe_logger (
        spdlog::level::info, "Inquiry: sr={} Hz, buf_size={}, channels={}", sr, buf_size, n_ch);
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int Shimmer3::start_streaming_cmd ()
{
    int res = send_command (ShimmerBT::START_STREAMING_COMMAND);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
        return res;
    return wait_for_ack ();
}

int Shimmer3::stop_streaming_cmd ()
{
    int res = send_command (ShimmerBT::STOP_STREAMING_COMMAND);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
        return res;
    return wait_for_ack ();
}

int Shimmer3::disable_status_ack ()
{
    uint8_t args[1] = {0x00};
    int res = send_command_with_args (ShimmerBT::ENABLE_STATUS_ACK_COMMAND, args, 1);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
        return res;
    return wait_for_ack ();
}

int Shimmer3::get_all_calibration (ShimmerAllCalibration &cal)
{
    int res = send_command (ShimmerBT::GET_ALL_CALIBRATION_COMMAND);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
        return res;
    res = wait_for_ack ();
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
        return res;

    uint8_t resp_code = 0;
    res = serial_read_byte (resp_code);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
        return res;
    if (resp_code != ShimmerBT::ALL_CALIBRATION_RESPONSE)
    {
        safe_logger (spdlog::level::err, "Unexpected calibration response code");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }

    uint8_t cal_data[ShimmerBT::ALL_CALIBRATION_LEN];
    res = serial_read (cal_data, ShimmerBT::ALL_CALIBRATION_LEN);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
        return res;

    if (!cal.parse (cal_data, ShimmerBT::ALL_CALIBRATION_LEN))
    {
        safe_logger (spdlog::level::warn, "Failed to parse calibration data");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }

    safe_logger (spdlog::level::info, "Calibration data retrieved");
    return (int)BrainFlowExitCodes::STATUS_OK;
}


// ---------------------------------------------------------------------------
// Packet parsing helpers
// ---------------------------------------------------------------------------

bool Shimmer3::lookup_channel_dtype (EChannelType ch, ShimmerChannelDType &out)
{
    for (int i = 0; i < CH_DTYPE_TABLE_COUNT; i++)
    {
        if (CH_DTYPE_TABLE[i].channel == ch && CH_DTYPE_TABLE[i].valid)
        {
            out = CH_DTYPE_TABLE[i].dtype;
            return true;
        }
    }
    return false;
}

// The device always sends a 3-byte timestamp at the start of each
// data packet, followed by the channels reported in the inquiry.
// We prepend a TIMESTAMP entry so the parsing loop can handle
// everything uniformly.
void Shimmer3::build_active_channel_list (const std::vector<EChannelType> &inquiry_channels)
{
    active_channels.clear ();
    active_dtypes.clear ();

    ShimmerChannelDType ts_dtype = {Shimmer3Const::TIMESTAMP_SIZE, false, true};
    active_channels.push_back (EChannelType::TIMESTAMP);
    active_dtypes.push_back (ts_dtype);

    for (auto ch : inquiry_channels)
    {
        ShimmerChannelDType dtype;
        if (lookup_channel_dtype (ch, dtype))
        {
            active_channels.push_back (ch);
            active_dtypes.push_back (dtype);
        }
        else
        {
            safe_logger (spdlog::level::warn, "No dtype for channel 0x{:02X}, skipping",
                static_cast<int> (ch));
        }
    }
}

int Shimmer3::compute_packet_size ()
{
    int total = 0;
    for (auto &dt : active_dtypes)
        total += dt.size;
    return total;
}


// ---------------------------------------------------------------------------
// Board interface: prepare_session
// ---------------------------------------------------------------------------

int Shimmer3::prepare_session ()
{
    if (initialized)
    {
        safe_logger (spdlog::level::info, "Session already prepared");
        return (int)BrainFlowExitCodes::STATUS_OK;
    }

    if (params.serial_port.empty ())
    {
        safe_logger (spdlog::level::err, "A serial port path is required (Bluetooth SPP)");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    port_name = params.serial_port;

    serial_port = Serial::create (port_name.c_str (), this);
    int res = serial_port->open_serial_port ();
    if (res < 0)
    {
        safe_logger (spdlog::level::err, "Could not open serial port {}", port_name);
        delete serial_port;
        serial_port = NULL;
        return (int)BrainFlowExitCodes::UNABLE_TO_OPEN_PORT_ERROR;
    }

    serial_port->set_serial_port_settings (1000, false);

    // Give the Bluetooth link a moment to settle
    std::this_thread::sleep_for (std::chrono::milliseconds (500));

    // Check that we can talk to the device
    uint16_t fw_type = 0, fw_major = 0;
    uint8_t fw_minor = 0, fw_rel = 0;
    res = get_firmware_version (fw_type, fw_major, fw_minor, fw_rel);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "Could not read firmware version");
        serial_port->close_serial_port ();
        delete serial_port;
        serial_port = NULL;
        return res;
    }

    // Turn off periodic status ACKs so they don't clutter the stream.
    // Older firmware may not support this, which is fine.
    res = disable_status_ack ();
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        safe_logger (
            spdlog::level::warn, "Could not disable status ACK — old firmware? Continuing anyway");
    }

    // Ask the device which channels are currently enabled
    double sr = 0.0;
    int buf_sz = 0;
    std::vector<EChannelType> inquiry_channels;
    res = send_inquiry (sr, buf_sz, inquiry_channels);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "Inquiry failed");
        serial_port->close_serial_port ();
        delete serial_port;
        serial_port = NULL;
        return res;
    }

    sampling_rate = sr;
    build_active_channel_list (inquiry_channels);
    packet_size = compute_packet_size ();

    safe_logger (spdlog::level::info, "Active channels: {}, packet size: {} bytes",
        active_channels.size (), packet_size);

    // Try to grab calibration data. If it fails we'll just
    // pass through raw values. It's not ideal but still usable.
    res = get_all_calibration (calibration);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        safe_logger (
            spdlog::level::warn, "Could not retrieve calibration data; raw values will be used");
    }

    initialized = true;
    return (int)BrainFlowExitCodes::STATUS_OK;
}


// ---------------------------------------------------------------------------
// Board interface: start_stream / stop_stream / release_session
// ---------------------------------------------------------------------------

int Shimmer3::start_stream (int buffer_size, const char *streamer_params)
{
    if (!initialized)
    {
        safe_logger (spdlog::level::err, "Call prepare_session first");
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    if (keep_alive)
    {
        safe_logger (spdlog::level::err, "Streaming thread is already running");
        return (int)BrainFlowExitCodes::STREAM_ALREADY_RUN_ERROR;
    }

    int res = prepare_for_acquisition (buffer_size, streamer_params);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
        return res;

    res = start_streaming_cmd ();
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "Start-streaming command failed");
        return res;
    }

    keep_alive = true;
    streaming_thread = std::thread ([this] { this->read_thread (); });

    return (int)BrainFlowExitCodes::STATUS_OK;
}

int Shimmer3::stop_stream ()
{
    if (keep_alive)
    {
        keep_alive = false;
        streaming_thread.join ();

        int res = stop_streaming_cmd ();
        if (res != (int)BrainFlowExitCodes::STATUS_OK)
            safe_logger (spdlog::level::warn, "Stop-streaming command failed");

        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    return (int)BrainFlowExitCodes::STREAM_THREAD_IS_NOT_RUNNING;
}

int Shimmer3::release_session ()
{
    if (initialized)
    {
        if (keep_alive)
            stop_stream ();

        free_packages ();
        initialized = false;

        if (serial_port != NULL)
        {
            serial_port->close_serial_port ();
            delete serial_port;
            serial_port = NULL;
        }

        active_channels.clear ();
        active_dtypes.clear ();
        packet_size = 0;
        sampling_rate = 0.0;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}


// ---------------------------------------------------------------------------
// Board interface: config_board
//
// Accepted commands:
//   "set_sampling_rate:<Hz>"       — change the sampling rate
//   "set_sensors:<hex_bitfield>"   — change which sensors are enabled
// ---------------------------------------------------------------------------

int Shimmer3::config_board (std::string config, std::string &response)
{
    if (!initialized)
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;

    if (config.rfind ("set_sampling_rate:", 0) == 0)
    {
        double sr = std::stod (config.substr (18));
        int res = set_sampling_rate (sr);
        if (res == (int)BrainFlowExitCodes::STATUS_OK)
        {
            sampling_rate = sr;
            response = "OK";
        }
        return res;
    }
    else if (config.rfind ("set_sensors:", 0) == 0)
    {
        uint32_t bitfield = static_cast<uint32_t> (std::stoul (config.substr (12), nullptr, 16));
        int res = set_sensors (bitfield);
        if (res == (int)BrainFlowExitCodes::STATUS_OK)
        {
            // The channel layout may have changed, so re-query
            double sr = 0.0;
            int buf_sz = 0;
            std::vector<EChannelType> inquiry_channels;
            res = send_inquiry (sr, buf_sz, inquiry_channels);
            if (res == (int)BrainFlowExitCodes::STATUS_OK)
            {
                sampling_rate = sr;
                build_active_channel_list (inquiry_channels);
                packet_size = compute_packet_size ();
                response = "OK";
            }
        }
        return res;
    }

    safe_logger (spdlog::level::warn, "Unrecognised config command: {}", config);
    response = "UNKNOWN_COMMAND";
    return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
}


// ---------------------------------------------------------------------------
// Streaming thread
//
// Runs in the background after start_stream(). Reads one data packet
// at a time from the serial port, decodes each channel, and pushes
// the result into BrainFlow's ring buffer.
// ---------------------------------------------------------------------------

void Shimmer3::read_thread ()
{
    int num_rows = board_descr["default"]["num_rows"];
    std::vector<uint8_t> pkt_buf (packet_size);

    while (keep_alive)
    {
        // Every data packet starts with a 0x00 header byte
        uint8_t header = 0xFF;
        int res = serial_read_byte (header);
        if (res != (int)BrainFlowExitCodes::STATUS_OK)
        {
            safe_logger (spdlog::level::warn, "Read error in streaming thread");
            continue;
        }

        if (header != ShimmerBT::DATA_PACKET)
        {
            // Stale in-stream status response (consume and discard it)
            if (header == ShimmerBT::INSTREAM_CMD_RESPONSE)
            {
                uint8_t discard;
                serial_read_byte (discard);
            }
            continue;
        }

        res = serial_read (pkt_buf.data (), packet_size);
        if (res != (int)BrainFlowExitCodes::STATUS_OK)
        {
            safe_logger (spdlog::level::warn, "Incomplete data packet");
            continue;
        }

        // Decode each channel and place it in the right row
        double *package = new double[num_rows];
        for (int i = 0; i < num_rows; i++)
            package[i] = 0.0;

        int offset = 0;
        for (size_t ch_idx = 0; ch_idx < active_channels.size (); ch_idx++)
        {
            if (offset + active_dtypes[ch_idx].size > packet_size)
            {
                safe_logger (spdlog::level::err, "Packet overrun while parsing");
                break;
            }

            int32_t raw_val = active_dtypes[ch_idx].decode (pkt_buf.data () + offset);
            offset += active_dtypes[ch_idx].size;

            EChannelType ch = active_channels[ch_idx];

            switch (ch)
            {
                case EChannelType::TIMESTAMP:
                {
                    double ts_sec = static_cast<double> (static_cast<uint32_t> (raw_val)) /
                        Shimmer3Const::DEV_CLOCK_RATE;
                    if (board_descr["default"].contains ("timestamp_channel"))
                    {
                        int ts_row = board_descr["default"]["timestamp_channel"];
                        package[ts_row] = ts_sec;
                    }
                    break;
                }

                case EChannelType::ACCEL_LN_X:
                case EChannelType::ACCEL_LN_Y:
                case EChannelType::ACCEL_LN_Z:
                case EChannelType::ACCEL_WR_X:
                case EChannelType::ACCEL_WR_Y:
                case EChannelType::ACCEL_WR_Z:
                {
                    if (board_descr["default"].contains ("accel_channels"))
                    {
                        auto &rows = board_descr["default"]["accel_channels"];
                        int axis = -1;
                        if (ch == EChannelType::ACCEL_LN_X || ch == EChannelType::ACCEL_WR_X)
                            axis = 0;
                        else if (ch == EChannelType::ACCEL_LN_Y || ch == EChannelType::ACCEL_WR_Y)
                            axis = 1;
                        else if (ch == EChannelType::ACCEL_LN_Z || ch == EChannelType::ACCEL_WR_Z)
                            axis = 2;
                        if (axis >= 0 && axis < (int)rows.size ())
                            package[rows[axis].get<int> ()] = static_cast<double> (raw_val);
                    }
                    break;
                }

                case EChannelType::GYRO_X:
                case EChannelType::GYRO_Y:
                case EChannelType::GYRO_Z:
                {
                    if (board_descr["default"].contains ("gyro_channels"))
                    {
                        auto &rows = board_descr["default"]["gyro_channels"];
                        int axis = static_cast<int> (ch) - static_cast<int> (EChannelType::GYRO_X);
                        if (axis >= 0 && axis < (int)rows.size ())
                            package[rows[axis].get<int> ()] = static_cast<double> (raw_val);
                    }
                    break;
                }

                case EChannelType::MAG_REG_X:
                case EChannelType::MAG_REG_Y:
                case EChannelType::MAG_REG_Z:
                {
                    if (board_descr["default"].contains ("magnetometer_channels"))
                    {
                        auto &rows = board_descr["default"]["magnetometer_channels"];
                        int axis =
                            static_cast<int> (ch) - static_cast<int> (EChannelType::MAG_REG_X);
                        if (axis >= 0 && axis < (int)rows.size ())
                            package[rows[axis].get<int> ()] = static_cast<double> (raw_val);
                    }
                    break;
                }

                case EChannelType::GSR_RAW:
                {
                    if (board_descr["default"].contains ("eda_channels"))
                    {
                        auto &rows = board_descr["default"]["eda_channels"];
                        if (!rows.empty ())
                            package[rows[0].get<int> ()] = static_cast<double> (raw_val);
                    }
                    break;
                }

                case EChannelType::EXG1_CH1_24BIT:
                case EChannelType::EXG1_CH2_24BIT:
                case EChannelType::EXG1_CH1_16BIT:
                case EChannelType::EXG1_CH2_16BIT:
                case EChannelType::EXG2_CH1_24BIT:
                case EChannelType::EXG2_CH2_24BIT:
                case EChannelType::EXG2_CH1_16BIT:
                case EChannelType::EXG2_CH2_16BIT:
                {
                    if (board_descr["default"].contains ("ecg_channels"))
                    {
                        auto &rows = board_descr["default"]["ecg_channels"];
                        // Figure out which ExG data channel this is by
                        // counting how many we've already seen.
                        int exg_idx = 0;
                        for (size_t k = 0; k < ch_idx; k++)
                        {
                            auto prev = active_channels[k];
                            if (prev == EChannelType::EXG1_CH1_24BIT ||
                                prev == EChannelType::EXG1_CH2_24BIT ||
                                prev == EChannelType::EXG1_CH1_16BIT ||
                                prev == EChannelType::EXG1_CH2_16BIT ||
                                prev == EChannelType::EXG2_CH1_24BIT ||
                                prev == EChannelType::EXG2_CH2_24BIT ||
                                prev == EChannelType::EXG2_CH1_16BIT ||
                                prev == EChannelType::EXG2_CH2_16BIT)
                                exg_idx++;
                        }
                        if (exg_idx < (int)rows.size ())
                            package[rows[exg_idx].get<int> ()] = static_cast<double> (raw_val);
                    }
                    break;
                }

                case EChannelType::TEMPERATURE:
                {
                    if (board_descr["default"].contains ("temperature_channels"))
                    {
                        auto &rows = board_descr["default"]["temperature_channels"];
                        if (!rows.empty ())
                            package[rows[0].get<int> ()] = static_cast<double> (raw_val);
                    }
                    break;
                }

                case EChannelType::VBATT:
                {
                    if (board_descr["default"].contains ("battery_channel"))
                    {
                        int row = board_descr["default"]["battery_channel"];
                        package[row] = static_cast<double> (raw_val);
                    }
                    break;
                }

                default:
                {
                    // Anything else (ADC, pressure, strain, ExG status, …)
                    // goes into other_channels if the board description has them.
                    if (board_descr["default"].contains ("other_channels"))
                    {
                        auto &rows = board_descr["default"]["other_channels"];
                        static int other_idx = 0;
                        if (other_idx < (int)rows.size ())
                        {
                            package[rows[other_idx].get<int> ()] = static_cast<double> (raw_val);
                            other_idx++;
                        }
                    }
                    break;
                }
            }
        }

        // Host-side wall-clock timestamp in the last row
        if (board_descr["default"].contains ("timestamp_channel"))
        {
            int last_row = num_rows - 1;
            package[last_row] = get_timestamp ();
        }

        push_package (package);
        delete[] package;
    }
}
