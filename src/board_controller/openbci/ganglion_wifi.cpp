#include <vector>

#include "custom_cast.h"
#include "ganglion_wifi.h"
#include "timestamp.h"

#include "http.h"

#ifndef _WIN32
#include <errno.h>
#endif

#define START_BYTE 0xA0
#define END_BYTE_STANDARD 0xC0
#define END_BYTE_ANALOG 0xC1
#define END_BYTE_MAX 0xC6


int GanglionWifi::config_board (std::string conf, std::string &response)
{
    const char *config = conf.c_str ();
    if ((config == NULL) || (strlen (config) == 0))
    {
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    if ((keep_alive) && ((config[0] == 'z') || (config[0] == 'Z')))
    {
        safe_logger (
            spdlog::level::err, "For Ganglion WIFI impedance works only if streaming is stopped");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    if (config[0] == 'z')
    {
        is_cheking_impedance = true;
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    if (config[0] == 'Z')
    {
        is_cheking_impedance = false;
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    return send_config (config);
}

int GanglionWifi::start_stream (int buffer_size, const char *streamer_params)
{
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

    // need to start impedance streaming
    if (is_cheking_impedance)
    {
        res = send_config ("z");
        if (res == (int)BrainFlowExitCodes::STATUS_OK)
        {
            keep_alive = true;
            streaming_thread = std::thread ([this] { this->read_thread_impedance (); });
            return (int)BrainFlowExitCodes::STATUS_OK;
        }
        else
        {
            return res;
        }
    }
    // start plain streaming
    else
    {
        std::string url = "http://" + params.ip_address + "/stream/start";
        http_t *request = http_get (url.c_str (), NULL);
        if (!request)
        {
            safe_logger (spdlog::level::err, "error during request creation, to {}", url.c_str ());
            return (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
        int send_res = wait_for_http_resp (request);
        if (send_res != (int)BrainFlowExitCodes::STATUS_OK)
        {
            http_release (request);
            return send_res;
        }
        http_release (request);

        keep_alive = true;
        streaming_thread = std::thread ([this] { this->read_thread (); });
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int GanglionWifi::stop_stream ()
{
    if (keep_alive)
    {
        keep_alive = false;
        streaming_thread.join ();
        // Board fails to receive/handle "Z" almost all the time
        // workaround - still send 'Z' and send /stream/stop after that, maybe it will work
        if (is_cheking_impedance)
        {
            send_config ("Z");
        }
        std::string url = "http://" + params.ip_address + "/stream/stop";
        http_t *request = http_get (url.c_str (), NULL);
        if (!request)
        {
            safe_logger (spdlog::level::err, "error during request creation, to {}", url.c_str ());
            return (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
        int send_res = wait_for_http_resp (request);
        if (send_res != (int)BrainFlowExitCodes::STATUS_OK)
        {
            http_release (request);
            return send_res;
        }
        http_release (request);

        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    else
    {
        return (int)BrainFlowExitCodes::STREAM_THREAD_IS_NOT_RUNNING;
    }
}

void GanglionWifi::read_thread ()
{
    /*  Only for Ganglion + WIFI shield, plain Ganglion has another format
        Byte 1: 0xA0
        Byte 2: Sample Number
        Bytes 3-5: Data value for EEG channel 1
        Bytes 6-8: Data value for EEG channel 2
        Bytes 9-11: Data value for EEG channel 3
        Bytes 12-14: Data value for EEG channel 4
        Bytes 15-26: zeroes
        Aux Data Bytes 27-32: 6 bytes of data
        Byte 33: 0xCX where X is 0-F in hex
    */
    int res;
    unsigned char b[OpenBCIWifiShieldBoard::package_size];
    int num_rows = board_descr["num_rows"];
    double *package = new double[num_rows];
    for (int i = 0; i < num_rows; i++)
    {
        package[i] = 0.0;
    }
    std::vector<int> eeg_channels = board_descr["eeg_channels"];

    while (keep_alive)
    {
        // check start byte
        res = server_socket->recv (b, OpenBCIWifiShieldBoard::package_size);
        if (res != OpenBCIWifiShieldBoard::package_size)
        {
            if (res < 0)
            {
#ifdef _WIN32
                safe_logger (spdlog::level::warn, "WSAGetLastError is {}", WSAGetLastError ());
#else
                safe_logger (spdlog::level::warn, "errno {} message {}", errno, strerror (errno));
#endif
            }

            continue;
        }

        if (b[0] != START_BYTE)
        {
            continue;
        }
        if ((b[32] < END_BYTE_STANDARD) || (b[32] > END_BYTE_MAX))
        {
            safe_logger (spdlog::level::warn, "Wrong end byte, found {}", b[32]);
            continue;
        }

        // package num
        package[board_descr["package_num_channel"].get<int> ()] = (double)b[1];
        // eeg
        for (unsigned int i = 0; i < eeg_channels.size (); i++)
        {
            package[eeg_channels[i]] = eeg_scale * cast_24bit_to_int32 (b + 2 + 3 * i);
        }
        // end byte
        package[board_descr["other_channels"][0].get<int> ()] = (double)b[32];
        // place raw bytes to other_channels with end byte
        package[board_descr["other_channels"][1].get<int> ()] = (double)b[26];
        package[board_descr["other_channels"][2].get<int> ()] = (double)b[27];
        package[board_descr["other_channels"][3].get<int> ()] = (double)b[28];
        package[board_descr["other_channels"][4].get<int> ()] = (double)b[29];
        package[board_descr["other_channels"][5].get<int> ()] = (double)b[30];
        package[board_descr["other_channels"][6].get<int> ()] = (double)b[31];
        // place accel data
        if (b[32] == END_BYTE_STANDARD)
        {
            // accel
            // mistake in firmware in axis
            package[board_descr["accel_channels"][0].get<int> ()] =
                accel_scale * cast_16bit_to_int32 (b + 28);
            package[board_descr["accel_channels"][1].get<int> ()] =
                accel_scale * cast_16bit_to_int32 (b + 26);
            package[board_descr["accel_channels"][2].get<int> ()] =
                -accel_scale * cast_16bit_to_int32 (b + 30);
        }
        // place analog data
        if (b[32] == END_BYTE_ANALOG)
        {
            // analog
            package[board_descr["analog_channels"][0].get<int> ()] = cast_16bit_to_int32 (b + 26);
            package[board_descr["analog_channels"][1].get<int> ()] = cast_16bit_to_int32 (b + 28);
            package[board_descr["analog_channels"][2].get<int> ()] = cast_16bit_to_int32 (b + 30);
        }

        package[board_descr["timestamp_channel"].get<int> ()] = get_timestamp ();
        push_package (package);
    }
    delete[] package;
}

void GanglionWifi::read_thread_impedance ()
{
    int res;
    unsigned char b[OpenBCIWifiShieldBoard::package_size];
    int num_channels = 0;
    get_num_rows (board_id, &num_channels);
    double *package = new double[num_channels];
    for (int i = 0; i < num_channels; i++)
    {
        package[i] = 0.0;
    }

    while (keep_alive)
    {
        // check start byte
        res = server_socket->recv (b, OpenBCIWifiShieldBoard::package_size);
        if (res != OpenBCIWifiShieldBoard::package_size)
        {
            safe_logger (spdlog::level::warn, "recv result: {}", res);
            if (res == -1)
            {
#ifdef _WIN32
                safe_logger (spdlog::level::warn, "WSAGetLastError is {}", WSAGetLastError ());
#else
                safe_logger (spdlog::level::warn, "errno {} message {}", errno, strerror (errno));
#endif
            }

            continue;
        }

        if (b[0] != START_BYTE)
        {
            continue;
        }
        int channel_num = (int)(b[1]);
        // parsing resistance value for channel as asci string
        int val = 0;
        int i = 0;
        // find first non zero byte
        for (i = 20; i > 2; i--)
        {
            if (b[i] != 0)
            {
                break;
            }
        }
        std::string asci_value ((const char *)(b + 2), i - 1);
        try
        {
            val = std::stoi (asci_value);
        }
        catch (...)
        {
            safe_logger (
                spdlog::level::err, "failed to parse impedance data: {}", asci_value.c_str ());
            continue;
        }

        // channel_num starts from 1, first resistance channel is 18
        safe_logger (
            spdlog::level::trace, "resistance value is {}, channel number is {}", val, channel_num);
        if (!((channel_num > 0) && (channel_num < 6)))
        {
            safe_logger (spdlog::level::warn, "channel number is {}", channel_num);
            continue;
        }
        package[18 + channel_num - 1] = val;

        package[23] = get_timestamp ();
        push_package (package);
    }
    delete[] package;
}
