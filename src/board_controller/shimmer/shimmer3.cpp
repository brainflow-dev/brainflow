#include <chrono>
#include <cmath>
#include <cstdint>
#include <exception>
#include <string>
#include <vector>

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
    samples_per_packet = 1;
    configured_gsr_range = 0xFF;
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

int Shimmer3::read_exact (uint8_t *buf, int len, bool cancellable)
{
    if (serial_port == nullptr)
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    if ((buf == nullptr) || (len < 0))
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    if (len == 0)
        return (int)BrainFlowExitCodes::STATUS_OK;

    int got = 0;
    int idle = 0;
    const int max_idle = 2;

    while (got < len)
    {
        if (cancellable && !keep_alive)
            return (int)BrainFlowExitCodes::STREAM_THREAD_IS_NOT_RUNNING;

        int res =
            serial_port->read_from_serial_port (reinterpret_cast<char *> (buf + got), len - got);

        if (cancellable && !keep_alive)
            return (int)BrainFlowExitCodes::STREAM_THREAD_IS_NOT_RUNNING;

        if (res > 0)
        {
            got += res;
            idle = 0;
        }
        else if (res == 0)
        {
            if (++idle > max_idle)
            {
                safe_logger (spdlog::level::err, "serial read timeout {}/{}", got, len);
                return (int)BrainFlowExitCodes::GENERAL_ERROR;
            }
        }
        else
        {
            safe_logger (spdlog::level::err, "serial read error {}", res);
            return (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
    }

    return (int)BrainFlowExitCodes::STATUS_OK;
}

int Shimmer3::read_byte (uint8_t &out, bool cancellable)
{
    return read_exact (&out, 1, cancellable);
}

// Read bytes until ACK. In-stream responses may arrive first.
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
            uint8_t discard = 0;
            res = read_byte (discard);
            if (res != (int)BrainFlowExitCodes::STATUS_OK)
                return res;
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

// Disable the ACK prefix for in-stream command responses.
int Shimmer3::cmd_disable_instream_ack_prefix ()
{
    uint8_t buf[2] = {Opcode::SET_INSTREAM_RESPONSE_ACK_PREFIX_STATE, 0x00};
    int res = write_bytes (buf, 2);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
        return res;
    return wait_for_ack ();
}

int Shimmer3::cmd_set_sensors (uint32_t bitfield)
{
    if (bitfield > 0x00FFFFFFU)
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;

    uint8_t buf[4] = {Opcode::SET_SENSORS_COMMAND, static_cast<uint8_t> (bitfield & 0xFFU),
        static_cast<uint8_t> ((bitfield >> 8) & 0xFFU),
        static_cast<uint8_t> ((bitfield >> 16) & 0xFFU)};

    int res = write_bytes (buf, 4);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
        return res;
    return wait_for_ack ();
}

int Shimmer3::cmd_set_sampling_rate (double hz)
{
    if (!std::isfinite (hz) || (hz <= 0.0))
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;

    double divider_value = CLOCK_HZ / hz;
    if ((divider_value < 1.0) || (divider_value > 65535.0))
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;

    uint16_t div = static_cast<uint16_t> (std::lround (divider_value));
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

// INQUIRY returns the rate, configuration, packet buffer size and channel IDs.
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
    if (div == 0)
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;

    sampling_rate = divider_to_hz (div);

    uint32_t config_setup = static_cast<uint32_t> (hdr[2]) | (static_cast<uint32_t> (hdr[3]) << 8) |
        (static_cast<uint32_t> (hdr[4]) << 16) | (static_cast<uint32_t> (hdr[5]) << 24);

    configured_gsr_range = static_cast<uint8_t> ((config_setup >> 25) & 0x07U);

    uint8_t num_channels = hdr[6];
    samples_per_packet = hdr[7];
    if (samples_per_packet == 0)
    {
        safe_logger (spdlog::level::err, "inquiry returned buffer size 0");
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }

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

    if (!build_packet_layout (signals))
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;

    if (packet_data_size <= 0)
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;

    safe_logger (spdlog::level::info,
        "inquiry: {} Hz, {} channels, buffer_size {}, sample data {} bytes", sampling_rate,
        (int)num_channels, (int)samples_per_packet, packet_data_size);
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
    return write_bytes (&cmd, 1);
}

// ---------------------------------------------------------------------------
// Packet layout
// ---------------------------------------------------------------------------

// Each sample contains a timestamp followed by the active channels.
bool Shimmer3::build_packet_layout (const std::vector<Signal> &signals)
{
    packet_layout.clear ();
    packet_data_size = 0;

    bool analog_accel = false;
    bool digital_accel = false;
    bool mpu_accel = false;
    bool lsm_mag = false;
    bool mpu_mag = false;

    for (Signal s : signals)
    {
        switch (s)
        {
            case Signal::ACCEL_LN_X:
            case Signal::ACCEL_LN_Y:
            case Signal::ACCEL_LN_Z:
                analog_accel = true;
                break;

            case Signal::ACCEL_WR_X:
            case Signal::ACCEL_WR_Y:
            case Signal::ACCEL_WR_Z:
                digital_accel = true;
                break;

            case Signal::MPU_ACCEL_X:
            case Signal::MPU_ACCEL_Y:
            case Signal::MPU_ACCEL_Z:
                mpu_accel = true;
                break;

            case Signal::MAG_X:
            case Signal::MAG_Y:
            case Signal::MAG_Z:
                lsm_mag = true;
                break;

            case Signal::MPU_MAG_X:
            case Signal::MPU_MAG_Y:
            case Signal::MPU_MAG_Z:
                mpu_mag = true;
                break;

            default:
                break;
        }
    }

    int accel_sources = static_cast<int> (analog_accel) + static_cast<int> (digital_accel) +
        static_cast<int> (mpu_accel);

    int mag_sources = static_cast<int> (lsm_mag) + static_cast<int> (mpu_mag);

    if ((accel_sources > 1) || (mag_sources > 1))
    {
        safe_logger (spdlog::level::err, "multiple IMU sources use the same BrainFlow rows");
        return false;
    }

    bool found = false;
    FieldFormat fmt = format_for (Signal::TIMESTAMP, found);
    if (!found || (fmt.width == 0))
        return false;

    packet_layout.push_back ({Signal::TIMESTAMP, fmt});
    packet_data_size += fmt.width;

    for (Signal s : signals)
    {
        fmt = format_for (s, found);
        if (!found || (fmt.width == 0))
        {
            safe_logger (spdlog::level::err, "unsupported signal 0x{:02X}", (int)s);
            packet_layout.clear ();
            packet_data_size = 0;
            return false;
        }

        packet_layout.push_back ({s, fmt});
        packet_data_size += fmt.width;
    }

    return true;
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
    if (serial_port == nullptr)
        return (int)BrainFlowExitCodes::UNABLE_TO_OPEN_PORT_ERROR;

    int serial_res = serial_port->open_serial_port ();
    if (serial_res < 0)
    {
        safe_logger (spdlog::level::err, "failed to open {}", port_name);
        delete serial_port;
        serial_port = nullptr;
        return (int)BrainFlowExitCodes::UNABLE_TO_OPEN_PORT_ERROR;
    }

    serial_res = serial_port->set_serial_port_settings (1000, false);
    if (serial_res != (int)SerialExitCodes::OK)
    {
        safe_logger (
            spdlog::level::err, "failed to configure {}, serial error {}", port_name, serial_res);
        serial_port->close_serial_port ();
        delete serial_port;
        serial_port = nullptr;
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }

    std::this_thread::sleep_for (std::chrono::milliseconds (500));

    // Confirm this is a Shimmer3 and reject Shimmer3R.
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

    if (cmd_get_fw_version () != (int)BrainFlowExitCodes::STATUS_OK)
        safe_logger (spdlog::level::warn, "could not read firmware version");

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
    if ((packet_data_size <= 0) || packet_layout.empty () || (samples_per_packet == 0))
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    if (buffer_size <= 0)
        return (int)BrainFlowExitCodes::INVALID_BUFFER_SIZE_ERROR;

    int res = prepare_for_acquisition (buffer_size, streamer_params);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
        return res;

    package_num = 0.0;
    {
        std::lock_guard<std::mutex> lk (sync_mutex);
        first_data_received = false;
    }

    res = cmd_start_streaming ();
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to start streaming");
        free_packages ();
        return res;
    }

    keep_alive = true;
    streaming_thread = std::thread ([this] { this->read_thread (); });

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
        free_packages ();
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

    int res = cmd_stop_streaming ();

    // Drain trailing data and ACK.
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

    return res;
}

int Shimmer3::release_session ()
{
    if (keep_alive)
        stop_stream ();
    else if (streaming_thread.joinable ())
        streaming_thread.join ();

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
    samples_per_packet = 1;
    configured_gsr_range = 0xFF;
    sampling_rate = 0.0;
    package_num = 0.0;

    {
        std::lock_guard<std::mutex> lk (sync_mutex);
        first_data_received = false;
    }

    return (int)BrainFlowExitCodes::STATUS_OK;
}

int Shimmer3::get_board_sampling_rate (int preset)
{
    if (preset != (int)BrainFlowPresets::DEFAULT_PRESET)
        return Board::get_board_sampling_rate (preset);
    if (!std::isfinite (sampling_rate) || (sampling_rate <= 0.0))
        return Board::get_board_sampling_rate (preset);
    return static_cast<int> (std::lround (sampling_rate));
}

// Supported config strings:
//   "sampling_rate:<Hz>"
//   "sensors:<hex24>"
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

    try
    {
        if (config.rfind ("sampling_rate:", 0) == 0)
        {
            std::string value = config.substr (14);
            if (value.empty ())
            {
                response = "INVALID_SAMPLING_RATE";
                return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
            }

            size_t parsed = 0;
            double hz = std::stod (value, &parsed);
            if ((parsed != value.size ()) || !std::isfinite (hz) || (hz <= 0.0))
            {
                response = "INVALID_SAMPLING_RATE";
                return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
            }

            double divider_value = CLOCK_HZ / hz;
            if ((divider_value < 1.0) || (divider_value > 65535.0))
            {
                response = "INVALID_SAMPLING_RATE";
                return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
            }

            int res = cmd_set_sampling_rate (hz);
            if (res != (int)BrainFlowExitCodes::STATUS_OK)
                return res;
            return reinquire (response);
        }

        if (config.rfind ("sensors:", 0) == 0)
        {
            std::string value = config.substr (8);
            if (value.empty ())
            {
                response = "INVALID_SENSORS";
                return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
            }

            size_t parsed = 0;
            unsigned long bits = std::stoul (value, &parsed, 16);

            if ((parsed != value.size ()) || (bits > 0x00FFFFFFUL))
            {
                response = "INVALID_SENSORS";
                return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
            }

            uint32_t sensor_bits = static_cast<uint32_t> (bits);
            uint32_t accel_bits = sensor_bits & ACCEL_SENSORS;
            uint32_t mag_bits = sensor_bits & MAG_SENSORS;
            uint32_t exg1_bits = sensor_bits & EXG1_SENSORS;
            uint32_t exg2_bits = sensor_bits & EXG2_SENSORS;

            if (((sensor_bits & ~SUPPORTED_SENSORS) != 0) ||
                ((accel_bits & (accel_bits - 1)) != 0) || ((mag_bits & (mag_bits - 1)) != 0) ||
                ((exg1_bits & (exg1_bits - 1)) != 0) || ((exg2_bits & (exg2_bits - 1)) != 0))
            {
                response = "INVALID_SENSORS";
                return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
            }

            int res = cmd_set_sensors (sensor_bits);
            if (res != (int)BrainFlowExitCodes::STATUS_OK)
                return res;
            return reinquire (response);
        }
    }
    catch (const std::exception &e)
    {
        safe_logger (spdlog::level::err, "invalid config '{}': {}", config, e.what ());
        response = "INVALID_CONFIG";
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    safe_logger (spdlog::level::warn, "unknown config '{}'", config);
    response = "UNKNOWN_COMMAND";
    return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
}

// ---------------------------------------------------------------------------
// GSR conversion
// ---------------------------------------------------------------------------

bool Shimmer3::convert_gsr_to_microsiemens (int32_t raw_value, double &eda_us) const
{
    uint16_t raw = static_cast<uint16_t> (raw_value & 0xFFFF);
    uint16_t adc = raw & 0x0FFF;
    uint8_t range = configured_gsr_range;

    if (range == 4)
        range = static_cast<uint8_t> ((raw >> 14) & 0x03);

    if (range > 3)
    {
        eda_us = 0.0;
        return false;
    }

    struct Coeff
    {
        double p1;
        double p2;
    };

    static constexpr Coeff coeffs[4] = {
        {0.0373, -24.9915}, {0.0054, -3.5194}, {0.0015, -1.0163}, {0.0004558, -0.3014}};

    eda_us = coeffs[range].p1 * adc + coeffs[range].p2;
    if (!std::isfinite (eda_us) || (eda_us <= 0.0))
        eda_us = 0.0;
    return true;
}

// ---------------------------------------------------------------------------
// Routing decoded values to BrainFlow rows
// ---------------------------------------------------------------------------

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

void Shimmer3::route_field (Signal s, int32_t raw, double *package, int &other_idx)
{
    const json &d = board_descr["default"];

    auto put_row = [&] (const char *key, int index, double value)
    {
        auto rows = rows_by_key (d, key);
        if ((index >= 0) && (index < (int)rows.size ()))
            package[rows[index]] = value;
    };

    switch (s)
    {
        case Signal::TIMESTAMP:
            break;

        case Signal::ACCEL_LN_X:
        case Signal::ACCEL_WR_X:
            put_row ("accel_channels", 0, raw);
            break;

        case Signal::ACCEL_LN_Y:
        case Signal::ACCEL_WR_Y:
            put_row ("accel_channels", 1, raw);
            break;

        case Signal::ACCEL_LN_Z:
        case Signal::ACCEL_WR_Z:
            put_row ("accel_channels", 2, raw);
            break;

        case Signal::GYRO_X:
            put_row ("gyro_channels", 0, raw);
            break;

        case Signal::GYRO_Y:
            put_row ("gyro_channels", 1, raw);
            break;

        case Signal::GYRO_Z:
            put_row ("gyro_channels", 2, raw);
            break;

        case Signal::MAG_X:
            put_row ("magnetometer_channels", 0, raw);
            break;

        case Signal::MAG_Y:
            put_row ("magnetometer_channels", 1, raw);
            break;

        case Signal::MAG_Z:
            put_row ("magnetometer_channels", 2, raw);
            break;

        case Signal::GSR:
        {
            double eda_us = 0.0;
            if (!convert_gsr_to_microsiemens (raw, eda_us))
                safe_logger (spdlog::level::warn, "invalid configured GSR range {}",
                    (int)configured_gsr_range);
            put_row ("eda_channels", 0, eda_us);
            break;
        }

        case Signal::TEMPERATURE:
            put_row ("temperature_channels", 0, raw);
            break;

        case Signal::VBATT:
            if (d.contains ("battery_channel"))
                package[d["battery_channel"].get<int> ()] = static_cast<double> (raw);
            break;

        case Signal::EXG1_CH1_24BIT:
        case Signal::EXG1_CH1_16BIT:
            put_row ("ecg_channels", 0, raw);
            break;

        case Signal::EXG1_CH2_24BIT:
        case Signal::EXG1_CH2_16BIT:
            put_row ("ecg_channels", 1, raw);
            break;

        case Signal::EXG2_CH1_24BIT:
        case Signal::EXG2_CH1_16BIT:
            put_row ("ecg_channels", 2, raw);
            break;

        case Signal::EXG2_CH2_24BIT:
        case Signal::EXG2_CH2_16BIT:
            put_row ("ecg_channels", 3, raw);
            break;

        case Signal::MPU_ACCEL_X:
            put_row ("accel_channels", 0, raw);
            break;

        case Signal::MPU_ACCEL_Y:
            put_row ("accel_channels", 1, raw);
            break;

        case Signal::MPU_ACCEL_Z:
            put_row ("accel_channels", 2, raw);
            break;

        case Signal::MPU_MAG_X:
            put_row ("magnetometer_channels", 0, raw);
            break;

        case Signal::MPU_MAG_Y:
            put_row ("magnetometer_channels", 1, raw);
            break;

        case Signal::MPU_MAG_Z:
            put_row ("magnetometer_channels", 2, raw);
            break;

        default:
        {
            auto rows = rows_by_key (d, "other_channels");
            if (other_idx < (int)rows.size ())
                package[rows[other_idx]] = static_cast<double> (raw);
            other_idx++;
            break;
        }
    }
}

// ---------------------------------------------------------------------------
// Streaming thread
// ---------------------------------------------------------------------------

void Shimmer3::read_thread ()
{
    const json &d = board_descr["default"];
    int num_rows = d["num_rows"];
    int payload_size = packet_data_size * (int)samples_per_packet;
    std::vector<uint8_t> buf (payload_size);

    int ts_row = -1;
    if (d.contains ("timestamp_channel"))
        ts_row = d["timestamp_channel"];

    int package_num_row = -1;
    if (d.contains ("package_num_channel"))
        package_num_row = d["package_num_channel"];

    while (keep_alive)
    {
        uint8_t header = 0xFF;
        int res = read_byte (header, true);
        if (res != (int)BrainFlowExitCodes::STATUS_OK)
        {
            if (!keep_alive)
                break;
            continue;
        }

        if (header != Opcode::DATA_PACKET)
        {
            if (header == Opcode::INSTREAM_CMD_RESPONSE)
            {
                uint8_t discard = 0;
                read_byte (discard, true);
            }
            continue;
        }

        res = read_exact (buf.data (), payload_size, true);
        if (res != (int)BrainFlowExitCodes::STATUS_OK)
        {
            if (!keep_alive)
                break;
            continue;
        }

        double packet_timestamp = get_timestamp ();

        for (int sample = 0; sample < (int)samples_per_packet; ++sample)
        {
            std::vector<double> package (num_rows, 0.0);

            int offset = sample * packet_data_size;
            int sample_end = offset + packet_data_size;
            int other_idx = 0;
            bool ok = true;

            for (const auto &field : packet_layout)
            {
                if ((field.format.width <= 0) || (offset + field.format.width > sample_end))
                {
                    ok = false;
                    break;
                }

                int32_t raw = decode_field (field.format, buf.data () + offset);
                offset += field.format.width;

                if (field.signal == Signal::TIMESTAMP)
                    continue;

                route_field (field.signal, raw, package.data (), other_idx);
            }

            if (!ok || (offset != sample_end))
            {
                continue;
            }

            if (package_num_row >= 0)
                package[package_num_row] = package_num;
            package_num += 1.0;

            if (ts_row >= 0)
            {
                int samples_after = (int)samples_per_packet - sample - 1;
                package[ts_row] =
                    packet_timestamp - static_cast<double> (samples_after) / sampling_rate;
            }

            push_package (package.data ());

            bool notify = false;
            {
                std::lock_guard<std::mutex> lk (sync_mutex);
                if (!first_data_received)
                {
                    first_data_received = true;
                    notify = true;
                }
            }
            if (notify)
                sync_cv.notify_one ();
        }
    }
}
