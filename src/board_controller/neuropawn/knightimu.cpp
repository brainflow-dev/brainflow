#include <cstdint>
#include <limits>
#include <math.h>
#include <string.h>
#include <vector>

#include "custom_cast.h"
#include "json.hpp"
#include "knightimu.h"
#include "serial.h"
#include "timestamp.h"

using json = nlohmann::json;

constexpr int KnightIMU::start_byte;
constexpr int KnightIMU::end_byte;
const std::set<int> KnightIMU::allowed_gains = {1, 2, 3, 4, 6, 8, 12};

KnightIMU::KnightIMU (int board_id, struct BrainFlowInputParams params) : Board (board_id, params)
{
    serial = NULL;
    is_streaming = false;
    keep_alive = false;
    initialized = false;
    gain = 12; // default gain value

    // Parse gain from other_info if provided
    if (!params.other_info.empty ())
    {
        try
        {
            json j = json::parse (params.other_info);
            if (j.contains ("gain"))
            {
                int parsed_gain = j["gain"];
                // Validate gain is one of allowed values
                if (allowed_gains.count (parsed_gain))
                {
                    gain = parsed_gain;
                    safe_logger (spdlog::level::info, "KnightIMU board gain set to {}", gain);
                }
                else
                {
                    safe_logger (spdlog::level::warn,
                        "Invalid gain value {} in other_info, using default 12", parsed_gain);
                }
            }
            else
            {
                safe_logger (spdlog::level::info, "No gain field in other_info, using default 12");
            }
        }
        catch (json::parse_error &e)
        {
            safe_logger (spdlog::level::warn,
                "Failed to parse JSON from other_info: {}, using default gain 12", e.what ());
        }
        catch (json::exception &e)
        {
            safe_logger (spdlog::level::warn,
                "JSON exception while parsing other_info: {}, using default gain 12", e.what ());
        }
    }
}

KnightIMU::~KnightIMU ()
{
    skip_logs = true;
    release_session ();
}

int KnightIMU::prepare_session ()
{
    if (initialized)
    {
        safe_logger (spdlog::level::info, "Session already prepared");
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    if (params.serial_port.empty ())
    {
        safe_logger (spdlog::level::err, "serial port is empty");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    serial = Serial::create (params.serial_port.c_str (), this);
    int port_open = open_port ();
    if (port_open != (int)BrainFlowExitCodes::STATUS_OK)
    {
        delete serial;
        serial = NULL;
        return port_open;
    }

    int set_settings = set_port_settings ();
    if (set_settings != (int)BrainFlowExitCodes::STATUS_OK)
    {
        delete serial;
        serial = NULL;
        return set_settings;
    }

    initialized = true;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int KnightIMU::start_stream (int buffer_size, const char *streamer_params)
{
    if (is_streaming)
    {
        safe_logger (spdlog::level::err, "Streaming thread already running");
        return (int)BrainFlowExitCodes::STREAM_ALREADY_RUN_ERROR;
    }
    int res = prepare_for_acquisition (buffer_size, streamer_params);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }

    serial->flush_buffer ();

    keep_alive = true;
    streaming_thread = std::thread ([this] { this->read_thread (); });
    is_streaming = true;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int KnightIMU::stop_stream ()
{
    if (is_streaming)
    {
        keep_alive = false;
        is_streaming = false;
        if (streaming_thread.joinable ())
        {
            streaming_thread.join ();
        }
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    else
    {
        return (int)BrainFlowExitCodes::STREAM_THREAD_IS_NOT_RUNNING;
    }
}

int KnightIMU::release_session ()
{
    if (initialized)
    {
        if (is_streaming)
        {
            stop_stream ();
        }
        free_packages ();
        initialized = false;
    }
    if (serial)
    {
        serial->close_serial_port ();
        delete serial;
        serial = NULL;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

void KnightIMU::read_thread ()
{
    /*
    Frame format (Arduino):
    [0]    1 Byte : START (0xA0)
    [1]    1 Byte : counter
    [2..17]16 Bytes: 8x EXG int16 (little-endian)
    [18]   1 Byte : LOFF STATP
    [19]   1 Byte : LOFF STATN
    [20..55]36 Bytes: 9x float32 IMU, little-endian: ax,ay,az,gx,gy,gz,mx,my,mz
    [56]   1 Byte : END (0xC0)
    */

    int res;
    constexpr int exg_channels_count = 8;
    constexpr int imu_channels_count = 9;
    constexpr int loff_bytes = 2;
    constexpr int frame_payload_size = 1 /*counter*/ + (exg_channels_count * 2) + loff_bytes +
        (imu_channels_count * 4) + 1 /*end*/;

    unsigned char b[frame_payload_size] = {0};

    float eeg_scale = 4 / float ((pow (2, 15) - 1)) / gain * 1000000.;
    int num_rows = board_descr["default"]["num_rows"];
    double *package = new double[num_rows];
    for (int i = 0; i < num_rows; i++)
    {
        package[i] = 0.0;
    }

    std::vector<int> eeg_channels = board_descr["default"]["eeg_channels"];
    std::vector<int> other_channels = board_descr["default"]["other_channels"];

    while (keep_alive)
    {
        // checking the start byte
        unsigned char start = 0;
        res = serial->read_from_serial_port (&start, 1);
        if (res != 1)
        {
            safe_logger (spdlog::level::debug, "unable to read 1 byte, {}");
            continue;
        }
        if (start != KnightIMU::start_byte)
        {
            continue;
        }

        int remaining_bytes = frame_payload_size;
        int pos = 0;
        while ((remaining_bytes > 0) && (keep_alive))
        {
            res = serial->read_from_serial_port (b + pos, remaining_bytes);
            if (res > 0)
            {
                remaining_bytes -= res;
                pos += res;
            }
        }

        if (!keep_alive)
        {
            break;
        }

        if (b[frame_payload_size - 1] != KnightIMU::end_byte)
        {
            safe_logger (spdlog::level::warn, "Wrong end byte {}", b[frame_payload_size - 1]);
            continue;
        }

        // package number / counter
        package[board_descr["default"]["package_num_channel"].get<int> ()] = (double)b[0];

        // exg data retrieval
        const int exg_offset = 1;
        for (unsigned int i = 0; i < eeg_channels.size () && i < exg_channels_count; i++)
        {
            package[eeg_channels[i]] = eeg_scale * cast_16bit_to_int32 (b + exg_offset + 2 * i);
        }

        // other channel data retrieval (keep old behavior)
        const int loff_offset = exg_offset + (exg_channels_count * 2);
        package[other_channels[0]] = (double)b[loff_offset];     // LOFF STATP
        package[other_channels[1]] = (double)b[loff_offset + 1]; // LOFF STATN

        // IMU float32, little-endian
        const int imu_offset = loff_offset + loff_bytes;
        for (int i = 0; i < imu_channels_count && (2 + i) < (int)other_channels.size (); i++)
        {
            uint32_t u = (uint32_t)b[imu_offset + 4 * i] |
                ((uint32_t)b[imu_offset + 4 * i + 1] << 8) |
                ((uint32_t)b[imu_offset + 4 * i + 2] << 16) |
                ((uint32_t)b[imu_offset + 4 * i + 3] << 24);
            float f = 0.0f;
            static_assert (sizeof (float) == 4, "float must be 4 bytes");
            std::memcpy (&f, &u, sizeof (f));
            package[other_channels[2 + i]] = (double)f;
        }

        // time stamp channel
        package[board_descr["default"]["timestamp_channel"].get<int> ()] = get_timestamp ();

        push_package (package);
    }
    delete[] package;
}

int KnightIMU::open_port ()
{
    if (serial->is_port_open ())
    {
        safe_logger (spdlog::level::err, "port {} already open", serial->get_port_name ());
        return (int)BrainFlowExitCodes::PORT_ALREADY_OPEN_ERROR;
    }

    safe_logger (spdlog::level::info, "openning port {}", serial->get_port_name ());
    int res = serial->open_serial_port ();
    if (res < 0)
    {
        return (int)BrainFlowExitCodes::UNABLE_TO_OPEN_PORT_ERROR;
    }
    safe_logger (spdlog::level::trace, "port {} is open", serial->get_port_name ());
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int KnightIMU::set_port_settings ()
{
    int res = serial->set_serial_port_settings (1000, false);
    if (res < 0)
    {
        safe_logger (spdlog::level::err, "Unable to set port settings, res is {}", res);
        return (int)BrainFlowExitCodes::SET_PORT_ERROR;
    }
    res = serial->set_custom_baudrate (115200);
    if (res < 0)
    {
        safe_logger (spdlog::level::err, "Unable to set custom baud rate, res is {}", res);
        return (int)BrainFlowExitCodes::SET_PORT_ERROR;
    }
    safe_logger (spdlog::level::trace, "set port settings");
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int KnightIMU::config_board (std::string config, std::string &response)
{
    if (!initialized)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    int res = (int)BrainFlowExitCodes::STATUS_OK;
    if (is_streaming)
    {
        safe_logger (spdlog::level::warn,
            "You are changing board params during streaming, it may lead to sync mismatch between "
            "data acquisition thread and device");
        res = send_to_board (config.c_str ());
    }
    else
    {
        // read response if streaming is not running
        res = send_to_board (config.c_str (), response);
    }

    return res;
}

int KnightIMU::send_to_board (const char *msg)
{
    int length = (int)strlen (msg);
    safe_logger (spdlog::level::debug, "sending {} to the board", msg);
    int res = serial->send_to_serial_port ((const void *)msg, length);
    if (res != length)
    {
        return (int)BrainFlowExitCodes::BOARD_WRITE_ERROR;
    }

    return (int)BrainFlowExitCodes::STATUS_OK;
}

int KnightIMU::send_to_board (const char *msg, std::string &response)
{
    int length = (int)strlen (msg);
    safe_logger (spdlog::level::debug, "sending {} to the board", msg);
    int res = serial->send_to_serial_port ((const void *)msg, length);
    if (res != length)
    {
        response = "";
        return (int)BrainFlowExitCodes::BOARD_WRITE_ERROR;
    }
    response = read_serial_response ();

    return (int)BrainFlowExitCodes::STATUS_OK;
}

std::string KnightIMU::read_serial_response ()
{
    constexpr int max_tmp_size = 4096;
    unsigned char tmp_array[max_tmp_size];
    unsigned char tmp;
    int tmp_id = 0;
    while (serial->read_from_serial_port (&tmp, 1) == 1)
    {
        if (tmp_id < max_tmp_size)
        {
            tmp_array[tmp_id] = tmp;
            tmp_id++;
        }
        else
        {
            serial->flush_buffer ();
            break;
        }
    }
    tmp_id = (tmp_id == max_tmp_size) ? tmp_id - 1 : tmp_id;
    tmp_array[tmp_id] = '\0';

    return std::string ((const char *)tmp_array);
}