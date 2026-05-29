#include <chrono>
#include <cstring>
#include <string>

#include "shimmer3.h"

#include "brainflow_constants.h"
#include "timestamp.h"

using namespace shimmer3;

Shimmer3::Shimmer3 (struct BrainFlowInputParams params)
    : Board ((int)BoardIds::SHIMMER3_BOARD, params)
{
    keep_alive = false;
    initialized = false;
    first_data_received = false;
    serial_port = nullptr;
    sampling_rate = 0.0;
    package_num = 0.0;
    packet_data_size = 0;
}


Shimmer3::~Shimmer3 ()
{
    skip_logs = true;
    release_session ();
}

// ---------------------------------------------------------------------------
// Serial helpers
// ---------------------------------------------------------------------------

int Shimmer3::write_bytes (const uint8_t *data, int len)
{
    if (serial_port == nullptr)
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;

    int res = serial_port->send_to_serial_port (reinterpret_cast<const void *> (data), len);
    if (res != len)
    {
        safe_logger (spdlog::level::err, "failed to write {} bytes", len);
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

// Block until len bytes are read or we give up.
int Shimmer3::read_exact (uint8_t *buf, int len)
{
    if (serial_port == nullptr)
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;

    int got = 0;
    int idle = 0;
    const int max_idle = 2000; // ~2s of empty reads before timing out
    while (got < len)
    {
        int r =
            serial_port->read_from_serial_port (reinterpret_cast<char *> (buf + got), len - got);
        if (r > 0)
        {
            got += r;
            idle = 0;
        }
        else if (r == 0)
        {
            if (++idle > max_idle)
            {
                safe_logger (spdlog::level::err, "serial read timeout {}/{}", got, len);
                return (int)BrainFlowExitCodes::GENERAL_ERROR;
            }
            std::this_thread::sleep_for (std::chrono::milliseconds (1));
        }
        else
        {
            safe_logger (spdlog::level::err, "serial read error");
            return (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int Shimmer3::read_byte (uint8_t &out)
{
    return read_exact (&out, 1);
}

// Read bytes until ACK (0xFF). In-stream command responses (0x8A) may arrive
// first; consume their following opcode byte and keep looking.
int Shimmer3::wait_for_ack ()
{
    for (int tries = 0; tries < 512; ++tries)
    {
        uint8_t b = 0;
        int res = read_byte (b);
        if (res != (int)BrainFlowExitCodes::STATUS_OK)
            return res;
        if (b == Opcode::ACK_COMMAND_PROCESSED)
            return (int)BrainFlowExitCodes::STATUS_OK;
        if (b == Opcode::INSTREAM_CMD_RESPONSE)
        {
            uint8_t discard;
            read_byte (discard);
        }
    }
    safe_logger (spdlog::level::err, "no ACK received");
    return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
}

// ---------------------------------------------------------------------------
// Device commands
// ---------------------------------------------------------------------------

int Shimmer3::cmd_get_fw_version ()
{
    uint8_t cmd = Opcode::GET_FW_VERSION_COMMAND;
    int res = write_bytes (&cmd, 1);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
        return res;
    res = wait_for_ack ();
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
        return res;

    uint8_t resp = 0;
    res = read_byte (resp);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
        return res;
    if (resp != Opcode::FW_VERSION_RESPONSE)
        return (int)BrainFlowExitCodes::GENERAL_ERROR;

    // 6 payload bytes: fw_id(2), major(2), minor(1), internal(1).
    uint8_t b[6];
    res = read_exact (b, 6);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
        return res;

    int fw_id = b[0] | (b[1] << 8);
    int major = b[2] | (b[3] << 8);
    safe_logger (spdlog::level::info, "Shimmer FW id={} v{}.{}.{}", fw_id, major, b[4], b[5]);
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int Shimmer3::cmd_get_hw_version (uint8_t &hw_version)
{
    uint8_t cmd = Opcode::GET_SHIMMER_VERSION_COMMAND;
    int res = write_bytes (&cmd, 1);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
        return res;
    res = wait_for_ack ();
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
        return res;

    uint8_t resp = 0;
    res = read_byte (resp);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
        return res;
    if (resp != Opcode::DEVICE_VERSION_RESPONSE)
        return (int)BrainFlowExitCodes::GENERAL_ERROR;

    return read_byte (hw_version);
}

// Ask the firmware not to prefix in-stream responses with an ACK byte, so the
// data stream stays clean. Older firmware may reject it; caller tolerates that.
int Shimmer3::cmd_disable_instream_ack_prefix ()
{
    uint8_t buf[2] = {Opcode::SET_INSTREAM_RESPONSE_ACK_PREFIX_STATE, 0x00};
    int res = write_bytes (buf, 2);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
        return res;
    return wait_for_ack ();
}

// SET_SENSORS_COMMAND + 3 little-endian bitfield bytes ("0x08, 0x80, 0x00, 0x00").
int Shimmer3::cmd_set_sensors (uint32_t bitfield)
{
    uint8_t buf[4] = {Opcode::SET_SENSORS_COMMAND, static_cast<uint8_t> (bitfield & 0xFF),
        static_cast<uint8_t> ((bitfield >> 8) & 0xFF),
        static_cast<uint8_t> ((bitfield >> 16) & 0xFF)};
    int res = write_bytes (buf, 4);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
        return res;
    return wait_for_ack ();
}

// SET_SAMPLING_RATE_COMMAND + 2 little-endian divider bytes.
int Shimmer3::cmd_set_sampling_rate (double hz)
{
    uint16_t div = hz_to_divider (hz);
    uint8_t buf[3] = {Opcode::SET_SAMPLING_RATE_COMMAND, static_cast<uint8_t> (div & 0xFF),
        static_cast<uint8_t> ((div >> 8) & 0xFF)};
    int res = write_bytes (buf, 3);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
        return res;
    res = wait_for_ack ();
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
        sampling_rate = divider_to_hz (div);
    return res;
}

// INQUIRY: learn the active sampling rate and the ordered list of channels so
// we can parse packets. Response layout (per BtStream manual, Table 5-1):
//   0x02 | rate(2) | config(4) | num_channels(1) | buffer_size(1) | chan IDs...
int Shimmer3::cmd_inquiry ()
{
    uint8_t cmd = Opcode::INQUIRY_COMMAND;
    int res = write_bytes (&cmd, 1);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
        return res;
    res = wait_for_ack ();
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
        return res;

    uint8_t resp = 0;
    res = read_byte (resp);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
        return res;
    if (resp != Opcode::INQUIRY_RESPONSE)
    {
        safe_logger (spdlog::level::err, "expected inquiry response, got 0x{:02X}", resp);
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }

    uint8_t hdr[8];
    res = read_exact (hdr, 8);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
        return res;

    uint16_t div = static_cast<uint16_t> (hdr[0] | (hdr[1] << 8));
    sampling_rate = divider_to_hz (div);
    uint8_t num_channels = hdr[6];
    uint8_t buffer_size = hdr[7];

    std::vector<uint8_t> ids (num_channels);
    if (num_channels > 0)
    {
        res = read_exact (ids.data (), num_channels);
        if (res != (int)BrainFlowExitCodes::STATUS_OK)
            return res;
    }

    std::vector<Signal> signals;
    for (uint8_t id : ids)
        signals.push_back (static_cast<Signal> (id));
    build_packet_layout (signals);

    safe_logger (spdlog::level::info,
        "inquiry: {} Hz, {} channels, buffer_size {}, packet data {} bytes", sampling_rate,
        (int)num_channels, (int)buffer_size, packet_data_size);
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int Shimmer3::cmd_start_streaming ()
{
    uint8_t cmd = Opcode::START_STREAMING_COMMAND;
    int res = write_bytes (&cmd, 1);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
        return res;
    return wait_for_ack ();
}

int Shimmer3::cmd_stop_streaming ()
{
    uint8_t cmd = Opcode::STOP_STREAMING_COMMAND;
    return write_bytes (&cmd, 1); // ACK consumed opportunistically by reader
}

// ---------------------------------------------------------------------------
// Packet layout
// ---------------------------------------------------------------------------

// Every packet is: 0x00 header, 3-byte timestamp, then the active channels in
// inquiry order. We prepend a synthetic TIMESTAMP field so parsing is uniform.
void Shimmer3::build_packet_layout (const std::vector<Signal> &signals)
{
    packet_layout.clear ();
    bool found = false;

    packet_layout.push_back ({Signal::TIMESTAMP, format_for (Signal::TIMESTAMP, found)});

    for (Signal s : signals)
    {
        FieldFormat fmt = format_for (s, found);
        if (!found || fmt.width == 0)
        {
            safe_logger (spdlog::level::warn, "skipping unsupported signal 0x{:02X}", (int)s);
            continue;
        }
        packet_layout.push_back ({s, fmt});
    }

    packet_data_size = 0;
    for (const auto &f : packet_layout)
        packet_data_size += f.format.width;
}

// ---------------------------------------------------------------------------
// Board interface
// ---------------------------------------------------------------------------

int Shimmer3::prepare_session ()
{
    if (initialized)
        return (int)BrainFlowExitCodes::STATUS_OK;

    if (params.serial_port.empty ())
    {
        safe_logger (spdlog::level::err, "serial_port (Bluetooth SPP) must be provided");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    port_name = params.serial_port;

    serial_port = Serial::create (port_name.c_str (), this);
    if (serial_port->open_serial_port () < 0)
    {
        safe_logger (spdlog::level::err, "failed to open {}", port_name);
        delete serial_port;
        serial_port = nullptr;
        return (int)BrainFlowExitCodes::UNABLE_TO_OPEN_PORT_ERROR;
    }
    serial_port->set_serial_port_settings (1000, false);
    std::this_thread::sleep_for (std::chrono::milliseconds (500));

    // Confirm we are actually talking to a Shimmer3 (reject Shimmer3R).
    uint8_t hw = 0;
    int res = cmd_get_hw_version (hw);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "could not read hardware version");
        release_session ();
        return res;
    }
    if (hw != HW_VERSION_SHIMMER3)
    {
        safe_logger (spdlog::level::err,
            "unsupported hardware version {} (this driver supports Shimmer3 only)", (int)hw);
        release_session ();
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }

    cmd_get_fw_version (); // informational only

    // Best-effort: silence the in-stream ACK prefix.
    if (cmd_disable_instream_ack_prefix () != (int)BrainFlowExitCodes::STATUS_OK)
        safe_logger (spdlog::level::warn, "could not disable in-stream ACK prefix (old firmware?)");

    res = cmd_inquiry ();
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        release_session ();
        return res;
    }

    initialized = true;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int Shimmer3::start_stream (int buffer_size, const char *streamer_params)
{
    if (!initialized)
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    if (keep_alive)
        return (int)BrainFlowExitCodes::STREAM_ALREADY_RUN_ERROR;
    if (packet_data_size <= 0)
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    if (buffer_size <= 0)
        return (int)BrainFlowExitCodes::INVALID_BUFFER_SIZE_ERROR;

    int res = prepare_for_acquisition (buffer_size, streamer_params);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
        return res;

    package_num = 0.0;           // reset sequence counter for the new stream
    first_data_received = false; // reset handshake flag

    res = cmd_start_streaming ();
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to start streaming");
        return res;
    }

    keep_alive = true;
    streaming_thread = std::thread ([this] { this->read_thread (); });

    // Wait until the read thread confirms real 0x00 data packets are flowing.
    // The device already ACKed START_STREAMING above; this additionally guards
    // against a paired-but-dead Bluetooth link that ACKs but never streams.
    int timeout = params.timeout > 0 ? params.timeout : 5;
    std::unique_lock<std::mutex> lk (sync_mutex);
    bool got_data = sync_cv.wait_for (
        lk, std::chrono::seconds (timeout), [this] { return first_data_received; });
    if (!got_data)
    {
        lk.unlock ();
        safe_logger (
            spdlog::level::err, "no data received within {} sec of starting stream", timeout);
        keep_alive = false;
        if (streaming_thread.joinable ())
            streaming_thread.join ();
        cmd_stop_streaming ();
        return (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
    }

    return (int)BrainFlowExitCodes::STATUS_OK;
}


int Shimmer3::stop_stream ()
{
    if (!keep_alive)
        return (int)BrainFlowExitCodes::STREAM_THREAD_IS_NOT_RUNNING;

    keep_alive = false;
    if (streaming_thread.joinable ())
        streaming_thread.join ();

    cmd_stop_streaming ();
    // Drain whatever is left in the buffer (trailing data / ACK).
    if (serial_port != nullptr)
    {
        uint8_t junk[256];
        for (int i = 0; i < 8; ++i)
        {
            int r =
                serial_port->read_from_serial_port (reinterpret_cast<char *> (junk), sizeof (junk));
            if (r <= 0)
                break;
        }
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int Shimmer3::release_session ()
{
    if (keep_alive)
        stop_stream ();

    if (initialized)
        free_packages ();
    initialized = false;

    if (serial_port != nullptr)
    {
        serial_port->close_serial_port ();
        delete serial_port;
        serial_port = nullptr;
    }

    packet_layout.clear ();
    packet_data_size = 0;
    sampling_rate = 0.0;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

// Supported config strings:
//   "sampling_rate:<Hz>"  -> change sampling rate (re-runs inquiry)
//   "sensors:<hex24>"     -> change enabled-sensor bitfield (re-runs inquiry)
int Shimmer3::config_board (std::string config, std::string &response)
{
    if (!initialized)
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    if (keep_alive)
    {
        safe_logger (spdlog::level::err, "cannot configure while streaming");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    auto reinquire = [this] (std::string &resp)
    {
        int r = cmd_inquiry ();
        if (r == (int)BrainFlowExitCodes::STATUS_OK)
            resp = "OK";
        return r;
    };

    if (config.rfind ("sampling_rate:", 0) == 0)
    {
        double hz = std::stod (config.substr (14));
        int res = cmd_set_sampling_rate (hz);
        if (res != (int)BrainFlowExitCodes::STATUS_OK)
            return res;
        return reinquire (response);
    }
    if (config.rfind ("sensors:", 0) == 0)
    {
        uint32_t bits = static_cast<uint32_t> (std::stoul (config.substr (8), nullptr, 16));
        int res = cmd_set_sensors (bits);
        if (res != (int)BrainFlowExitCodes::STATUS_OK)
            return res;
        return reinquire (response);
    }

    safe_logger (spdlog::level::warn, "unknown config '{}'", config);
    response = "UNKNOWN_COMMAND";
    return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
}

// ---------------------------------------------------------------------------
// Routing decoded values to BrainFlow rows
// ---------------------------------------------------------------------------

// Returns the row arrays from board_descr by key, or an empty vector.
static std::vector<int> rows_by_key (const json &descr, const char *key)
{
    std::vector<int> out;
    if (descr.contains (key))
    {
        for (const auto &v : descr[key])
            out.push_back (v.get<int> ());
    }
    return out;
}

int Shimmer3::route_field (Signal s, int32_t raw, double *package, int &accel_axis, int &gyro_axis,
    int &mag_axis, int &exg_idx, int &other_idx)
{
    const json &d = board_descr["default"];

    auto put_axis = [&] (const char *key, int axis)
    {
        auto rows = rows_by_key (d, key);
        if (axis >= 0 && axis < (int)rows.size ())
            package[rows[axis]] = static_cast<double> (raw);
    };

    switch (s)
    {
        case Signal::TIMESTAMP:
            // Device timestamp converted to seconds; only stored if a generic
            // "device timestamp" style row exists, otherwise ignored here.
            break;

        case Signal::ACCEL_LN_X:
        case Signal::ACCEL_LN_Y:
        case Signal::ACCEL_LN_Z:
        case Signal::ACCEL_WR_X:
        case Signal::ACCEL_WR_Y:
        case Signal::ACCEL_WR_Z:
            put_axis ("accel_channels", accel_axis++ % 3);
            break;

        case Signal::GYRO_X:
        case Signal::GYRO_Y:
        case Signal::GYRO_Z:
            put_axis ("gyro_channels", gyro_axis++ % 3);
            break;

        case Signal::MAG_X:
        case Signal::MAG_Y:
        case Signal::MAG_Z:
            put_axis ("magnetometer_channels", mag_axis++ % 3);
            break;

        case Signal::GSR:
        {
            auto rows = rows_by_key (d, "eda_channels");
            if (!rows.empty ())
                package[rows[0]] = static_cast<double> (raw);
            break;
        }

        case Signal::TEMPERATURE:
        {
            auto rows = rows_by_key (d, "temperature_channels");
            if (!rows.empty ())
                package[rows[0]] = static_cast<double> (raw);
            break;
        }

        case Signal::VBATT:
            if (d.contains ("battery_channel"))
                package[d["battery_channel"].get<int> ()] = static_cast<double> (raw);
            break;

        case Signal::EXG1_CH1_24BIT:
        case Signal::EXG1_CH2_24BIT:
        case Signal::EXG2_CH1_24BIT:
        case Signal::EXG2_CH2_24BIT:
        case Signal::EXG1_CH1_16BIT:
        case Signal::EXG1_CH2_16BIT:
        case Signal::EXG2_CH1_16BIT:
        case Signal::EXG2_CH2_16BIT:
        {
            auto rows = rows_by_key (d, "ecg_channels");
            if (exg_idx < (int)rows.size ())
                package[rows[exg_idx]] = static_cast<double> (raw);
            exg_idx++;
            break;
        }

        default:
        {
            // ADC channels, pressure, strain, ExG status, etc.
            auto rows = rows_by_key (d, "other_channels");
            if (other_idx < (int)rows.size ())
                package[rows[other_idx]] = static_cast<double> (raw);
            other_idx++;
            break;
        }
    }
    return 0;
}

// ---------------------------------------------------------------------------
// Streaming thread: resync on 0x00 header, read one packet, decode, push.
// ---------------------------------------------------------------------------

void Shimmer3::read_thread ()
{
    int num_rows = board_descr["default"]["num_rows"];
    std::vector<uint8_t> buf (packet_data_size);

    int ts_row = -1;
    if (board_descr["default"].contains ("timestamp_channel"))
        ts_row = board_descr["default"]["timestamp_channel"];

    int package_num_row = -1;
    if (board_descr["default"].contains ("package_num_channel"))
        package_num_row = board_descr["default"]["package_num_channel"];

    while (keep_alive)
    {
        // Resync: every data packet starts with a 0x00 header byte.
        uint8_t header = 0xFF;
        if (read_byte (header) != (int)BrainFlowExitCodes::STATUS_OK)
            continue;
        if (header != Opcode::DATA_PACKET)
        {
            if (header == Opcode::INSTREAM_CMD_RESPONSE)
            {
                uint8_t discard;
                read_byte (discard);
            }
            continue;
        }

        if (read_exact (buf.data (), packet_data_size) != (int)BrainFlowExitCodes::STATUS_OK)
            continue;

        double *package = new double[num_rows];
        for (int i = 0; i < num_rows; ++i)
            package[i] = 0.0;

        int offset = 0;
        int accel_axis = 0, gyro_axis = 0, mag_axis = 0, exg_idx = 0, other_idx = 0;
        bool ok = true;

        for (const auto &field : packet_layout)
        {
            if (offset + field.format.width > packet_data_size)
            {
                ok = false;
                break;
            }
            int32_t raw = decode_field (field.format, buf.data () + offset);
            offset += field.format.width;

            if (field.signal == Signal::TIMESTAMP)
                continue;

            route_field (
                field.signal, raw, package, accel_axis, gyro_axis, mag_axis, exg_idx, other_idx);
        }

        if (!ok)
        {
            delete[] package;
            continue;
        }

        if (package_num_row >= 0)
            package[package_num_row] = package_num;
        package_num += 1.0;

        if (ts_row >= 0)
            package[ts_row] = get_timestamp ();

        push_package (package);
        delete[] package;

        // Signal start_stream that data is flowing (first valid packet only).
        if (!first_data_received)
        {
            {
                std::lock_guard<std::mutex> lk (sync_mutex);
                first_data_received = true;
            }
            sync_cv.notify_one ();
        }
    }
}
