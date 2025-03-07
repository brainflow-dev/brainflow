#include "biolistener.h"

#include <chrono>
#include <sstream>
#include <string.h>

#include "json.hpp"
#include "network_interfaces.h"
#include "timestamp.h"

#include "biolistener_defines.h"

using json = nlohmann::json;


template <size_t BIOLISTENER_DATA_CHANNELS_COUNT>
BioListener<BIOLISTENER_DATA_CHANNELS_COUNT>::BioListener (
    int board_id, struct BrainFlowInputParams params)
    : Board (board_id, params)
{
    control_socket = NULL;
    keep_alive = false;
    initialized = false;
    control_port = -1;
    data_port = -1;
    timestamp_offset = -1;

    packet_size = sizeof (data_packet<BIOLISTENER_DATA_CHANNELS_COUNT>);

    // Data channels gain to default value
    for (int i = 0; i < BIOLISTENER_DATA_CHANNELS_COUNT; i++)
    {
        channels_gain[i] = BIOLISTENER_DEFAULT_PGA_GAIN;
    }
}

template <size_t BIOLISTENER_DATA_CHANNELS_COUNT>
BioListener<BIOLISTENER_DATA_CHANNELS_COUNT>::~BioListener ()
{
    skip_logs = true;
    release_session ();
}

template <size_t BIOLISTENER_DATA_CHANNELS_COUNT>
int BioListener<BIOLISTENER_DATA_CHANNELS_COUNT>::prepare_session ()
{
    if (initialized)
    {
        safe_logger (spdlog::level::info, "Session is already prepared");
        return (int)BrainFlowExitCodes::STATUS_OK;
    }

    if (params.timeout < 2)
    {
        params.timeout = 3;
        safe_logger (spdlog::level::warn, "Timeout is too low, setting to 3 sec");
    }

    if (params.ip_address.empty ())
    {
        params.ip_address = "0.0.0.0";
        safe_logger (
            spdlog::level::warn, "IP address is not set, listening on all network interfaces");
    }

    if (params.ip_port <= 0)
    {
        params.ip_port = 12345;
        safe_logger (spdlog::level::warn, "IP port is not set, using default value 12345");
    }

    int res = create_control_connection ();

    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        res = wait_for_connection ();
    }
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        auto json_command = json {{"command", BIOLISTENER_COMMAND_RESET_ADC}};

        res = send_control_msg ((json_command.dump () + PACKET_DELIMITER_CSV).c_str ());

        std::this_thread::sleep_for (std::chrono::milliseconds (500));
    }

    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        if (control_socket != NULL)
        {
            delete control_socket;
            control_socket = NULL;
        }
    }
    else
    {
        initialized = true;
    }

    return res;
}

template <size_t BIOLISTENER_DATA_CHANNELS_COUNT>
int BioListener<BIOLISTENER_DATA_CHANNELS_COUNT>::config_board (
    std::string conf, std::string &response)
{
    return send_control_msg (conf.c_str ());
}

template <size_t BIOLISTENER_DATA_CHANNELS_COUNT>
int BioListener<BIOLISTENER_DATA_CHANNELS_COUNT>::start_stream (
    int buffer_size, const char *streamer_params)
{
    if (!initialized)
    {
        safe_logger (spdlog::level::err, "You need to call prepare_session before config_board");
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    if (keep_alive)
    {
        safe_logger (spdlog::level::err, "Streaming thread already running");
        return (int)BrainFlowExitCodes::STREAM_ALREADY_RUN_ERROR;
    }
    int res = prepare_for_acquisition (buffer_size, streamer_params);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }

    auto json_command = json {{"command", BIOLISTENER_COMMAND_START_SAMPLING}};
    res = send_control_msg ((json_command.dump () + PACKET_DELIMITER_CSV).c_str ());
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        keep_alive = true;
        streaming_thread = std::thread ([this] { read_thread (); });
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

template <size_t BIOLISTENER_DATA_CHANNELS_COUNT>
int BioListener<BIOLISTENER_DATA_CHANNELS_COUNT>::stop_stream ()
{
    if (keep_alive)
    {
        auto json_command = json {{"command", BIOLISTENER_COMMAND_STOP_SAMPLING}};
        if (send_control_msg ((json_command.dump () + PACKET_DELIMITER_CSV).c_str ()) !=
            (int)BrainFlowExitCodes::STATUS_OK)
        {
            safe_logger (spdlog::level::warn, "failed to stop stream");
        }

        keep_alive = false;
        streaming_thread.join ();
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    return (int)BrainFlowExitCodes::STREAM_THREAD_IS_NOT_RUNNING;
}

template <size_t BIOLISTENER_DATA_CHANNELS_COUNT>
int BioListener<BIOLISTENER_DATA_CHANNELS_COUNT>::release_session ()
{
    if (initialized)
    {
        if (keep_alive)
        {
            stop_stream ();
        }
        initialized = false;
        free_packages ();
        if (control_socket)
        {
            control_socket->close ();
            delete control_socket;
            control_socket = NULL;
        }
        control_port = -1;
        data_port = -1;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

template <size_t BIOLISTENER_DATA_CHANNELS_COUNT>
bool BioListener<BIOLISTENER_DATA_CHANNELS_COUNT>::parse_tcp_buffer (const char *buffer,
    size_t buffer_size, data_packet<BIOLISTENER_DATA_CHANNELS_COUNT> &parsed_packet)
{
    // Ensure the buffer size matches the packet size
    if (buffer_size != packet_size)
    {
        safe_logger (spdlog::level::trace, "Buffer size mismatch!");
        return false;
    }

    // Copy the raw buffer into the struct
    std::memcpy (&parsed_packet, buffer, packet_size);

    // Validate the header and footer
    if (parsed_packet.header != BIOLISTENER_DATA_PACKET_HEADER)
    {
        safe_logger (spdlog::level::trace, "Invalid header! Expected: 0xA0");
        return false;
    }

    if (parsed_packet.footer != BIOLISTENER_DATA_PACKET_FOOTER)
    {
        safe_logger (spdlog::level::trace, "Invalid footer! Expected: 0xC0");
        return false;
    }

    return true;
}


// Convert raw ADC code (two's complement) to voltage
// ref: reference voltage (X if milli volts output is needed, X * 1000 if micro volts output)
//      1.2V for ADS131M08, 2.5V for AD7771
// raw_code: raw ADC code
// pga_gain: gain of the PGA
// adc_resolution: resolution of the ADC (2^23 for ADS131M08, 2^24 for AD7771)
template <size_t BIOLISTENER_DATA_CHANNELS_COUNT>
double BioListener<BIOLISTENER_DATA_CHANNELS_COUNT>::data_to_volts (
    double ref, uint32_t raw_code, double pga_gain, double adc_resolution)
{
    // Calculate the resolution in millivolts
    double resolution = ref / (adc_resolution * pga_gain);

    // Compute the voltage in millivolts based on the raw ADC code
    if (raw_code <= 0x7FFFFF)
    { // Positive range
        return resolution * (double)raw_code;
    }
    else
    { // Negative range (two's complement)
        return (resolution * (double)raw_code) - (ref / pga_gain);
    }
}


template <size_t BIOLISTENER_DATA_CHANNELS_COUNT>
void BioListener<BIOLISTENER_DATA_CHANNELS_COUNT>::read_thread ()
{
    char message[sizeof (data_packet<BIOLISTENER_DATA_CHANNELS_COUNT>)];
    int num_rows = board_descr["default"]["num_rows"];
    double *package = new double[num_rows];
    for (int i = 0; i < num_rows; i++)
    {
        package[i] = 0.0;
    }

    bool first_data_packet_received = false;

    std::vector<int> eeg_channels = board_descr["default"]["eeg_channels"];
    std::vector<int> other_channels = board_descr["default"]["other_channels"];
    std::vector<int> accel_channels = board_descr["auxiliary"]["accel_channels"];
    std::vector<int> gyro_channels = board_descr["auxiliary"]["gyro_channels"];
    int temperature_channel = board_descr["auxiliary"]["temperature_channels"][0];

    while (keep_alive)
    {
        int bytes_recv =
            control_socket->recv (message, sizeof (data_packet<BIOLISTENER_DATA_CHANNELS_COUNT>));
        if (bytes_recv < 1)
        {
            safe_logger (spdlog::level::trace, "no data received");
            continue;
        }

        try
        {
            data_packet<BIOLISTENER_DATA_CHANNELS_COUNT> parsed_packet {};
            if (!parse_tcp_buffer (message, bytes_recv, parsed_packet))
            {
                safe_logger (spdlog::level::err, "Failed to parse data packet");
                continue;
            }

            if (!first_data_packet_received)
            {
                // ENHANCEMENT: can be replaced with more accurate timestamp based on ntp or similar
                timestamp_offset = get_timestamp () - ((double)parsed_packet.ts / 1000.0);

                // Check gain
                for (int i = 0; i < BIOLISTENER_DATA_CHANNELS_COUNT; i++)
                {
                    int sensor_id = parsed_packet.s_id;

                    double pga_gain;
                    {
                        std::lock_guard<std::mutex> lock (m_channels_gain);
                        pga_gain = channels_gain[i];
                    }

                    if (parsed_packet.s_id == BIOLISTENER_ADC_AD7771)
                    {
                        // 1-8 OK
                        if (pga_gain < 1 || pga_gain > 8)
                        {
                            safe_logger (spdlog::level::critical, "Invalid pga gain for AD7771: {}",
                                pga_gain);
                            break;
                        }
                    }
                    else if (parsed_packet.s_id == BIOLISTENER_ADC_ADS131M08)
                    {
                        // 1-128 OK
                        if (pga_gain < 1 || pga_gain > 128)
                        {
                            safe_logger (spdlog::level::critical,
                                "Invalid pga gain for ADS131M08: {}", pga_gain);
                            break;
                        }
                    }
                }

                first_data_packet_received = true;
            }

            if (parsed_packet.type == BIOLISTENER_DATA_PACKET_BIOSIGNALS)
            {
                package[board_descr["default"]["timestamp_channel"].template get<int> ()] =
                    timestamp_offset + ((double)parsed_packet.ts / 1000.0);
                package[board_descr["default"]["package_num_channel"].template get<int> ()] =
                    parsed_packet.n;

                static uint32_t package_num_channel_last = parsed_packet.n - 1;

                if (package_num_channel_last + 1 != parsed_packet.n)
                {
                    safe_logger (spdlog::level::err,
                        "Package num mismatch BIOLISTENER_DATA_PACKET_BIOSIGNALS! Lost: {} packets",
                        parsed_packet.n - package_num_channel_last + 1);
                }

                package_num_channel_last = parsed_packet.n;

                int sensor_id = parsed_packet.s_id;

                for (int i = 0; i < BIOLISTENER_DATA_CHANNELS_COUNT; i++)
                {
                    double pga_gain;
                    {
                        std::lock_guard<std::mutex> lock (m_channels_gain);
                        pga_gain = channels_gain[i];
                    }

                    if (sensor_id == BIOLISTENER_ADC_AD7771)
                    {
                        static const double ref_microV = 2500000.0;
                        static const double adc_resolution = 16777216.0;
                        package[eeg_channels[i]] = data_to_volts (ref_microV, parsed_packet.data[i],
                                                       pga_gain, adc_resolution) *
                            2.0;
                    }
                    else if (sensor_id == BIOLISTENER_ADC_ADS131M08)
                    {
                        static const double ref_microV = 1200000.0;
                        static const double adc_resolution = 16777216.0;
                        package[eeg_channels[i]] = data_to_volts (ref_microV, parsed_packet.data[i],
                                                       pga_gain, adc_resolution) *
                            2.0;
                    }
                    else
                    {
                        safe_logger (spdlog::level::err, "Unknown sensor id: {}", sensor_id);
                        return;
                    }
                }
                push_package (package);
            }
            else if (parsed_packet.type == BIOLISTENER_DATA_PACKET_IMU)
            {
                package[board_descr["auxiliary"]["timestamp_channel"].template get<int> ()] =
                    timestamp_offset + ((double)parsed_packet.ts / 1000.0);
                package[board_descr["auxiliary"]["package_num_channel"].template get<int> ()] =
                    parsed_packet.n;

                static uint32_t package_num_channel_last = parsed_packet.n - 1;

                if (package_num_channel_last + 1 != parsed_packet.n)
                {
                    safe_logger (spdlog::level::err,
                        "Package num mismatch BIOLISTENER_DATA_PACKET_IMU! Lost: {} packets",
                        parsed_packet.n - package_num_channel_last + 1);
                }

                package_num_channel_last = parsed_packet.n;

                for (int i = 0; i < 3; i++)
                {
                    package[accel_channels[i]] = UINT32_TO_FLOAT (parsed_packet.data[i]);
                    package[gyro_channels[i]] = UINT32_TO_FLOAT (parsed_packet.data[i + 3]);
                }

                package[temperature_channel] = UINT32_TO_FLOAT (parsed_packet.data[6]);
                package[board_descr["auxiliary"]["battery_channel"].template get<int> ()] =
                    UINT32_TO_FLOAT (parsed_packet.data[7]);

                push_package (package, (int)BrainFlowPresets::AUXILIARY_PRESET);
            }
            else
            {
                safe_logger (
                    spdlog::level::err, "Unknown data packet type: {}", parsed_packet.type);
            }
        }
        catch (json::parse_error &e)
        {
            safe_logger (spdlog::level::err, "Failed to parse json: {}", e.what ());
        }
    }

    delete[] package;
}

template <size_t BIOLISTENER_DATA_CHANNELS_COUNT>
int BioListener<BIOLISTENER_DATA_CHANNELS_COUNT>::create_control_connection ()
{
    char local_ip[80];

    strncpy (local_ip, params.ip_address.c_str (), sizeof (local_ip) - 1);
    local_ip[sizeof (local_ip) - 1] = '\0';
    safe_logger (spdlog::level::info, "local ip addr is {}", local_ip);

    int res = (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    for (int i = 0; i < BIOLISTENER_MAX_PORTS_TO_TRY_TILL_FREE_FOUND; i += 1)
    {
        control_port = params.ip_port + i;
        control_socket = new SocketServerTCP (local_ip, control_port, true);
        if (control_socket->bind () == ((int)SocketServerTCPReturnCodes::STATUS_OK))
        {
            safe_logger (spdlog::level::info, "use port {} for control", control_port);
            res = (int)BrainFlowExitCodes::STATUS_OK;
            break;
        }
        else
        {
            safe_logger (spdlog::level::warn, "failed to connect to {}", control_port);
        }
        control_socket->close ();
        delete control_socket;
        control_socket = NULL;
    }
    return res;
}

template <size_t BIOLISTENER_DATA_CHANNELS_COUNT>
int BioListener<BIOLISTENER_DATA_CHANNELS_COUNT>::send_control_msg (const char *msg)
{
    // should never happen
    if (control_port < 0)
    {
        safe_logger (spdlog::level::info, "ports for control are not set");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }

    // try to convert msg from json to string
    try
    {
        auto j = json::parse (msg);
        if (j.contains ("command") && j["command"] == BIOLISTENER_COMMAND_SET_ADC_CHANNEL_PGA)
        {
            int channel = j["channel"];
            double pga = j["pga"];
            if (channel >= 0 && channel < BIOLISTENER_DATA_CHANNELS_COUNT)
            {
                std::lock_guard<std::mutex> lock (m_channels_gain);
                channels_gain[channel] = pga;
                safe_logger (spdlog::level::info, "Set gain for channel: {}", channel);
                safe_logger (spdlog::level::info, "Gain: {}", pga);
            }
        }
    }
    catch (json::parse_error &e)
    {
        safe_logger (spdlog::level::err, "Failed to parse json: {}", e.what ());
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    catch (json::exception &e)
    {
        safe_logger (spdlog::level::err, "Failed to parse json: {}", e.what ());
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    catch (...)
    {
        safe_logger (spdlog::level::err, "Failed to parse json");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    // convert msg to string just by copying
    std::string package = msg;

    int res = (int)BrainFlowExitCodes::STATUS_OK;
    int bytes_send = control_socket->send (package.c_str (), (int)package.size ());
    if (bytes_send != (int)package.size ())
    {
        safe_logger (spdlog::level::err, "failed to send control msg package: {}, res is {}", msg,
            bytes_send);
        res = (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    else
    {
        safe_logger (spdlog::level::info, "Message: {} sent", msg);
    }
    return res;
}

template <size_t BIOLISTENER_DATA_CHANNELS_COUNT>
int BioListener<BIOLISTENER_DATA_CHANNELS_COUNT>::wait_for_connection ()
{
    int res = (int)BrainFlowExitCodes::STATUS_OK;
    int accept_res = control_socket->accept ();
    if (accept_res != (int)SocketServerTCPReturnCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "error in accept");
        res = (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    else
    {
        int max_attempts = (params.timeout * 1000) / BIOLISTENER_SLEEP_TIME_BETWEEN_SOCKET_TRIES_MS;
        for (int i = 0; i < max_attempts; i++)
        {
            safe_logger (spdlog::level::trace, "waiting for accept {}/{}", i, max_attempts);
            if (control_socket->client_connected)
            {
                safe_logger (spdlog::level::trace, "BioListener connected");
                break;
            }
            else
            {
#ifdef _WIN32
                Sleep (BIOLISTENER_SLEEP_TIME_BETWEEN_SOCKET_TRIES_MS);
#else
                usleep (BIOLISTENER_SLEEP_TIME_BETWEEN_SOCKET_TRIES_MS * 1000);
#endif
            }
        }
        if (!control_socket->client_connected)
        {
            safe_logger (spdlog::level::trace, "BioListener - failed to establish connection");
            res = (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
        }
    }
    return res;
}

template class BioListener<8>;
template class BioListener<16>;
