#include "galea_v4.h"

#include <chrono>
#include <stdint.h>
#include <string.h>

#include <numeric>
#include <regex>
#include <sstream>

#include "custom_cast.h"
#include "json.hpp"
#include "timestamp.h"

using json = nlohmann::json;

#ifndef _WIN32
#include <errno.h>
#endif

constexpr int GaleaV4::package_size;
constexpr int GaleaV4::max_num_packages;
constexpr int GaleaV4::max_transaction_size;
constexpr int GaleaV4::socket_timeout;

GaleaV4::GaleaV4 (struct BrainFlowInputParams params)
    : Board ((int)BoardIds::GALEA_BOARD_V4, params)
{
    socket = NULL;
    is_streaming = false;
    keep_alive = false;
    initialized = false;
    state = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
    half_rtt = 0.0;
}

GaleaV4::~GaleaV4 ()
{
    skip_logs = true;
    release_session ();
}

int GaleaV4::prepare_session ()
{
    if (initialized)
    {
        safe_logger (spdlog::level::info, "Session is already prepared");
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    if ((params.timeout > 600) || (params.timeout < 1))
    {
        params.timeout = 5;
    }

    if (params.ip_address.empty ())
    {
        params.ip_address = find_device ();
        if (params.ip_address.empty ())
        {
            return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
        }
    }
    socket = new SocketClientUDP (params.ip_address.c_str (), 2390);
    int res = socket->connect ();
    if (res != (int)SocketClientUDPReturnCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to init socket: {}", res);
        delete socket;
        socket = NULL;
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }

    safe_logger (spdlog::level::trace, "timeout for socket is {}", socket_timeout);
    socket->set_timeout (socket_timeout);
    // force default settings for device
    std::string tmp;
    std::string default_settings = "d"; // use default mode
    res = config_board (default_settings, tmp);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to apply default settings");
        delete socket;
        socket = NULL;
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    // force default sampling rate - 250
    std::string sampl_rate = "~6";
    res = config_board (sampl_rate, tmp);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to apply default sampling rate");
        delete socket;
        socket = NULL;
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    initialized = true;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int GaleaV4::config_board (std::string conf, std::string &response)
{
    if (socket == NULL)
    {
        safe_logger (spdlog::level::err, "You need to call prepare_session before config_board");
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    // special handling for some commands
    if (conf == "calc_time")
    {
        if (is_streaming)
        {
            safe_logger (spdlog::level::err, "can not calc delay during the streaming.");
            return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
        }
        int res = calc_time (response);
        return res;
    }

    if (conf == "get_gains")
    {
        std::stringstream gains;

        for (int i = 0; i < 20; i++)
        {
            gains << gain_tracker.get_gain_for_channel (i);
            if (i < 19)
            {
                gains << ", ";
            }
        }
        response = gains.str ();
        safe_logger (spdlog::level::info, "gains for all channels: {}", response);
        return (int)BrainFlowExitCodes::STATUS_OK;
    }

    if (gain_tracker.apply_config (conf) == (int)OpenBCICommandTypes::INVALID_COMMAND)
    {
        safe_logger (spdlog::level::warn, "invalid command: {}", conf.c_str ());
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    const char *config = conf.c_str ();
    safe_logger (spdlog::level::debug, "Trying to config GaleaV4 with {}", config);
    int len = (int)strlen (config);
    int res = socket->send (config, len);
    if (len != res)
    {
        gain_tracker.revert_config ();
        if (res == -1)
        {
#ifdef _WIN32
            safe_logger (spdlog::level::err, "WSAGetLastError is {}", WSAGetLastError ());
#else
            safe_logger (spdlog::level::err, "errno {} message {}", errno, strerror (errno));
#endif
        }
        safe_logger (spdlog::level::err, "Failed to config a board");
        return (int)BrainFlowExitCodes::BOARD_WRITE_ERROR;
    }

    if (!is_streaming)
    {
        char b[GaleaV4::max_transaction_size];
        res = GaleaV4::max_transaction_size;
        int max_attempt = 25; // to dont get to infinite loop
        int current_attempt = 0;
        while ((res >= 0) && (res % GaleaV4::package_size == 0))
        {
            res = socket->recv (b, GaleaV4::max_transaction_size);
            if (res == -1)
            {
#ifdef _WIN32
                safe_logger (spdlog::level::err, "config_board recv ack WSAGetLastError is {}",
                    WSAGetLastError ());
#else
                safe_logger (spdlog::level::err, "config_board recv ack errno {} message {}", errno,
                    strerror (errno));
#endif
                return (int)BrainFlowExitCodes::BOARD_WRITE_ERROR;
            }
            current_attempt++;
            if (current_attempt == max_attempt)
            {
                safe_logger (spdlog::level::err, "Device is streaming data while it should not!");
                return (int)BrainFlowExitCodes::STREAM_ALREADY_RUN_ERROR;
            }
        }
        // set response string
        for (int i = 0; i < res; i++)
        {
            response = response + b[i];
        }
        switch (b[0])
        {
            case 'A':
                return (int)BrainFlowExitCodes::STATUS_OK;
            case 'I':
                safe_logger (spdlog::level::err, "invalid command");
                return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
            default:
                safe_logger (spdlog::level::warn, "unknown char received: {}", b[0]);
                return (int)BrainFlowExitCodes::STATUS_OK;
        }
    }
    else
    {
        safe_logger (spdlog::level::warn,
            "reconfiguring device during the streaming may lead to inconsistent data, it's "
            "recommended to call stop_stream before config_board");
    }

    return (int)BrainFlowExitCodes::STATUS_OK;
}

int GaleaV4::start_stream (int buffer_size, const char *streamer_params)
{
    if (!initialized)
    {
        safe_logger (spdlog::level::err, "You need to call prepare_session before config_board");
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    if (is_streaming)
    {
        safe_logger (spdlog::level::err, "Streaming thread already running");
        return (int)BrainFlowExitCodes::STREAM_ALREADY_RUN_ERROR;
    }

    // calc time before start stream
    std::string resp;
    for (int i = 0; i < 3; i++)
    {
        int res = calc_time (resp);
        if (res != (int)BrainFlowExitCodes::STATUS_OK)
        {
            return res;
        }
    }

    int res = prepare_for_acquisition (buffer_size, streamer_params);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }

    // start streaming
    res = socket->send ("b", 1);
    if (res != 1)
    {
        if (res == -1)
        {
#ifdef _WIN32
            safe_logger (spdlog::level::err, "WSAGetLastError is {}", WSAGetLastError ());
#else
            safe_logger (spdlog::level::err, "errno {} message {}", errno, strerror (errno));
#endif
        }
        safe_logger (spdlog::level::err, "Failed to send a command to board");
        return (int)BrainFlowExitCodes::BOARD_WRITE_ERROR;
    }

    keep_alive = true;
    streaming_thread = std::thread ([this] { this->read_thread (); });
    // wait for data to ensure that everything is okay
    std::unique_lock<std::mutex> lk (this->m);
    auto sec = std::chrono::seconds (1);
    if (cv.wait_for (lk, 3 * sec,
            [this] { return this->state != (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR; }))
    {
        this->is_streaming = true;
        return this->state;
    }
    else
    {
        safe_logger (spdlog::level::err, "no data received in 5sec, stopping thread");
        this->is_streaming = true;
        this->stop_stream ();
        return (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
    }
}

int GaleaV4::stop_stream ()
{
    if (is_streaming)
    {
        keep_alive = false;
        is_streaming = false;
        streaming_thread.join ();
        this->state = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
        int res = socket->send ("s", 1);
        if (res != 1)
        {
            if (res == -1)
            {
#ifdef _WIN32
                safe_logger (spdlog::level::err, "WSAGetLastError is {}", WSAGetLastError ());
#else
                safe_logger (spdlog::level::err, "errno {} message {}", errno, strerror (errno));
#endif
            }
            safe_logger (spdlog::level::err, "Failed to send a command to board");
            return (int)BrainFlowExitCodes::BOARD_WRITE_ERROR;
        }

        // free kernel buffer
        unsigned char b[GaleaV4::max_transaction_size];
        res = 0;
        int max_attempt = 25; // to dont get to infinite loop
        int current_attempt = 0;
        while (res != -1)
        {
            res = socket->recv (b, GaleaV4::max_transaction_size);
            current_attempt++;
            if (current_attempt == max_attempt)
            {
                safe_logger (
                    spdlog::level::err, "Command 's' was sent but streaming is still running.");
                return (int)BrainFlowExitCodes::BOARD_WRITE_ERROR;
            }
        }

        std::string resp;
        for (int i = 0; i < 3; i++)
        {
            res = calc_time (resp); // call it in the end once to print time in the end
            if (res != (int)BrainFlowExitCodes::STATUS_OK)
            {
                break; // dont send exit code
            }
        }
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    else
    {
        return (int)BrainFlowExitCodes::STREAM_THREAD_IS_NOT_RUNNING;
    }
}

int GaleaV4::release_session ()
{
    if (initialized)
    {
        if (is_streaming)
        {
            stop_stream ();
        }
        free_packages ();
        initialized = false;
        if (socket)
        {
            socket->close ();
            delete socket;
            socket = NULL;
        }
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

void GaleaV4::read_thread ()
{
    int res;
    unsigned char b[GaleaV4::max_transaction_size];
    DataBuffer time_buffer (1, 11);
    double latest_times[10];
    for (int i = 0; i < GaleaV4::max_transaction_size; i++)
    {
        b[i] = 0;
    }

    int num_exg_rows = board_descr["default"]["num_rows"];
    int num_aux_rows = board_descr["auxiliary"]["num_rows"];
    double *exg_package = new double[num_exg_rows];
    double *aux_package = new double[num_aux_rows];
    for (int i = 0; i < num_exg_rows; i++)
    {
        exg_package[i] = 0.0;
    }
    for (int i = 0; i < num_aux_rows; i++)
    {
        aux_package[i] = 0.0;
    }

    while (keep_alive)
    {
        res = socket->recv (b, GaleaV4::max_transaction_size);
        if (res == -1)
        {
#ifdef _WIN32
            safe_logger (spdlog::level::err, "WSAGetLastError is {}", WSAGetLastError ());
#else
            safe_logger (spdlog::level::err, "errno {} message {}", errno, strerror (errno));
#endif
            continue;
        }
        if (res % GaleaV4::package_size != 0)
        {
            if (res > 0)
            {
                // more likely its a string received, try to print it
                b[res] = '\0';
                safe_logger (spdlog::level::warn, "Received: {}", b);
            }
            continue;
        }
        else
        {
            int num_packages = res / GaleaV4::package_size;
            int offset_last_package = GaleaV4::package_size * (num_packages - 1);
            // calc delta between PC timestamp and device timestamp in last 10 packages,
            // use this delta later on to assign timestamps
            double pc_timestamp = get_timestamp ();
            unsigned long long timestamp_last_package = 0.0;
            memcpy (&timestamp_last_package, b + 88 + offset_last_package,
                sizeof (unsigned long long)); // microseconds
            double timestamp_last_package_converted =
                static_cast<double> (timestamp_last_package) / 1000000.0; // convert to seconds
            double time_delta = pc_timestamp - timestamp_last_package_converted;
            time_buffer.add_data (&time_delta);
            int num_time_deltas = (int)time_buffer.get_current_data (10, latest_times);
            time_delta = 0.0;
            for (int i = 0; i < num_time_deltas; i++)
            {
                time_delta += latest_times[i];
            }
            time_delta /= num_time_deltas;

            // inform main thread that everything is ok and first package was received
            if (this->state != (int)BrainFlowExitCodes::STATUS_OK)
            {
                safe_logger (spdlog::level::info,
                    "received first package with {} bytes streaming is started", res);
                {
                    std::lock_guard<std::mutex> lk (this->m);
                    this->state = (int)BrainFlowExitCodes::STATUS_OK;
                }
                this->cv.notify_one ();
                safe_logger (spdlog::level::debug, "start streaming");
            }

            for (int cur_package = 0; cur_package < num_packages; cur_package++)
            {
                int offset = cur_package * GaleaV4::package_size;
                // exg (default preset)
                exg_package[board_descr["default"]["package_num_channel"].get<int> ()] =
                    (double)b[0 + offset];
                for (int i = 4, tmp_counter = 0; i < 28; i++, tmp_counter++)
                {
                    double exg_scale = (double)(4.5 / float ((pow (2, 23) - 1)) /
                        gain_tracker.get_gain_for_channel (tmp_counter) * 1000000.);
                    exg_package[i - 3] =
                        exg_scale * (double)cast_24bit_to_int32 (b + offset + 5 + 3 * (i - 4));
                }
                unsigned long long timestamp_device = 0.0;
                memcpy (&timestamp_device, b + 88 + offset,
                    sizeof (unsigned long long)); // reports microseconds

                double timestamp_device_converted = static_cast<double> (timestamp_device);
                timestamp_device_converted /= 1000000.0; // convert to seconds

                exg_package[board_descr["default"]["timestamp_channel"].get<int> ()] =
                    timestamp_device_converted + time_delta - half_rtt;
                exg_package[board_descr["default"]["other_channels"][0].get<int> ()] = pc_timestamp;
                exg_package[board_descr["default"]["other_channels"][1].get<int> ()] =
                    timestamp_device_converted;
                push_package (exg_package);

                // aux, 5 times smaller sampling rate
                if (((int)b[0 + offset]) % 5 == 0)
                {
                    double accel_scale = (double)(8.0 / static_cast<double> (pow (2, 16) - 1));
                    double gyro_scale = (double)(1000.0 / static_cast<double> (pow (2, 16) - 1));
                    double magnetometer_scale_xy =
                        (double)(2.6 / static_cast<double> (pow (2, 13) - 1));
                    double magnetometer_scale_z =
                        (double)(5.0 / static_cast<double> (pow (2, 15) - 1));
                    aux_package[board_descr["auxiliary"]["package_num_channel"].get<int> ()] =
                        (double)b[0 + offset];
                    uint16_t temperature = 0;
                    int32_t ppg_ir = 0;
                    int32_t ppg_red = 0;
                    float eda;
                    memcpy (&temperature, b + 78 + offset, 2);
                    memcpy (&eda, b + 1 + offset, 4);
                    memcpy (&ppg_red, b + 80 + offset, 4);
                    memcpy (&ppg_ir, b + 84 + offset, 4);
                    // ppg
                    aux_package[board_descr["auxiliary"]["ppg_channels"][0].get<int> ()] =
                        (double)ppg_red;
                    aux_package[board_descr["auxiliary"]["ppg_channels"][1].get<int> ()] =
                        (double)ppg_ir;
                    // eda
                    aux_package[board_descr["auxiliary"]["eda_channels"][0].get<int> ()] =
                        (double)eda;
                    // temperature
                    aux_package[board_descr["auxiliary"]["temperature_channels"][0].get<int> ()] =
                        temperature / 100.0;
                    // battery
                    aux_package[board_descr["auxiliary"]["battery_channel"].get<int> ()] =
                        (double)b[77 + offset];
                    aux_package[board_descr["auxiliary"]["timestamp_channel"].get<int> ()] =
                        timestamp_device_converted + time_delta - half_rtt;
                    aux_package[board_descr["auxiliary"]["other_channels"][0].get<int> ()] =
                        pc_timestamp;
                    aux_package[board_descr["auxiliary"]["other_channels"][1].get<int> ()] =
                        timestamp_device_converted;
                    // accel
                    aux_package[board_descr["auxiliary"]["accel_channels"][0].get<int> ()] =
                        accel_scale * (double)cast_16bit_to_int32_swap_order (b + 96 + offset);
                    aux_package[board_descr["auxiliary"]["accel_channels"][1].get<int> ()] =
                        accel_scale * (double)cast_16bit_to_int32_swap_order (b + 98 + offset);
                    aux_package[board_descr["auxiliary"]["accel_channels"][2].get<int> ()] =
                        accel_scale * (double)cast_16bit_to_int32_swap_order (b + 100 + offset);
                    // gyro
                    aux_package[board_descr["auxiliary"]["gyro_channels"][0].get<int> ()] =
                        gyro_scale * (double)cast_16bit_to_int32_swap_order (b + 102 + offset);
                    aux_package[board_descr["auxiliary"]["gyro_channels"][1].get<int> ()] =
                        gyro_scale * (double)cast_16bit_to_int32_swap_order (b + 104 + offset);
                    aux_package[board_descr["auxiliary"]["gyro_channels"][2].get<int> ()] =
                        gyro_scale * (double)cast_16bit_to_int32_swap_order (b + 106 + offset);
                    // magnetometer
                    aux_package[board_descr["auxiliary"]["magnetometer_channels"][0].get<int> ()] =
                        magnetometer_scale_xy *
                        (double)cast_13bit_to_int32_swap_order (b + 108 + offset);
                    aux_package[board_descr["auxiliary"]["magnetometer_channels"][1].get<int> ()] =
                        magnetometer_scale_xy *
                        (double)cast_13bit_to_int32_swap_order (b + 110 + offset);
                    aux_package[board_descr["auxiliary"]["magnetometer_channels"][2].get<int> ()] =
                        magnetometer_scale_z *
                        (double)cast_15bit_to_int32_swap_order (b + 112 + offset);

                    push_package (aux_package, (int)BrainFlowPresets::AUXILIARY_PRESET);
                }
            }
        }
    }
    delete[] exg_package;
    delete[] aux_package;
}

int GaleaV4::calc_time (std::string &resp)
{
    constexpr int bytes_to_calc_rtt = 8;
    unsigned char b[bytes_to_calc_rtt];

    double start = get_timestamp ();
    int res = socket->send ("F4444444", bytes_to_calc_rtt);
    if (res != bytes_to_calc_rtt)
    {
        safe_logger (spdlog::level::warn, "failed to send time calc command to device");
        return (int)BrainFlowExitCodes::BOARD_WRITE_ERROR;
    }
    res = socket->recv (b, bytes_to_calc_rtt);
    double done = get_timestamp ();
    if (res != bytes_to_calc_rtt)
    {
        safe_logger (
            spdlog::level::warn, "failed to recv resp from time calc command, resp size {}", res);
        return (int)BrainFlowExitCodes::BOARD_WRITE_ERROR;
    }


    double duration = done - start;
    double timestamp_device = 0;
    memcpy (&timestamp_device, b, 8);
    timestamp_device /= 1000;
    half_rtt = duration / 2;

    json result;
    result["rtt"] = duration;
    result["timestamp_device"] = timestamp_device;
    result["pc_timestamp"] = start + half_rtt;

    resp = result.dump ();
    safe_logger (spdlog::level::info, "calc_time output: {}", resp);

    return (int)BrainFlowExitCodes::STATUS_OK;
}

std::string GaleaV4::find_device ()
{
#ifdef _WIN32
    std::string ssdp_ip_address = "192.168.137.255";
#else
    std::string ssdp_ip_address = "239.255.255.250";
#endif

    safe_logger (spdlog::level::trace, "trying to autodiscover device via SSDP");
    safe_logger (spdlog::level::trace, "timeout for search is {}", params.timeout);
    std::string ip_address = "";
    SocketClientUDP udp_client (ssdp_ip_address.c_str (),
        1900); // ssdp ip and port

    int res = udp_client.connect ();
    if (res == (int)SocketClientUDPReturnCodes::STATUS_OK)
    {
        std::string msearch = ("M-SEARCH * HTTP/1.1\r\nHost: " + ssdp_ip_address +
            ":1900\r\nMAN: ssdp:discover\r\n"
            "ST: urn:schemas-upnp-org:device:Basic:1\r\n"
            "MX: 3\r\n"
            "\r\n"
            "\r\n");

        safe_logger (spdlog::level::trace, "Use search request {}", msearch.c_str ());

        res = (int)udp_client.send (msearch.c_str (), (int)msearch.size ());
        if (res == msearch.size ())
        {
            unsigned char b[250];
            auto start_time = std::chrono::high_resolution_clock::now ();
            int run_time = 0;
            while (run_time < params.timeout)
            {
                res = udp_client.recv (b, 250);
                if (res > 1)
                {
                    std::string response ((const char *)b);
                    safe_logger (spdlog::level::trace, "Search response: {}", b);
                    std::regex rgx_ip ("LOCATION: http://([0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+)");
                    std::smatch matches;
                    if (std::regex_search (response, matches, rgx_ip) == true)
                    {
                        if (matches.size () == 2)
                        {
                            std::regex rgx_sn (
                                "USN: uuid:" + params.serial_number + "::upnp:rootdevice");
                            if ((params.serial_number.empty ()) ||
                                (std::regex_search (response, rgx_sn)))
                            {
                                ip_address = matches.str (1);
                                break;
                            }
                        }
                    }
                }
                auto end_time = std::chrono::high_resolution_clock::now ();
                run_time =
                    (int)std::chrono::duration_cast<std::chrono::seconds> (end_time - start_time)
                        .count ();
            }
        }
        else
        {
            safe_logger (spdlog::level::err, "Sent res {}", res);
        }
    }
    else
    {
        safe_logger (spdlog::level::err, "Failed to connect socket {}", res);
    }

    if (ip_address.empty ())
    {
        safe_logger (spdlog::level::err, "failed to find ip address");
    }
    else
    {
        safe_logger (spdlog::level::info, "use ip address {}", ip_address.c_str ());
    }

    udp_client.close ();
    return ip_address;
}