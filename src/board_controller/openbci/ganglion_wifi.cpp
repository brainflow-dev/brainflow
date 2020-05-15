#include "ganglion_wifi.h"
#include "custom_cast.h"
#include "timestamp.h"

#include "http.h"

#ifndef _WIN32
#include <errno.h>
#endif

#define START_BYTE 0xA0
#define END_BYTE_STANDARD 0xC0
#define END_BYTE_ANALOG 0xC1
#define END_BYTE_MAX 0xC6


int GanglionWifi::config_board (char *config)
{
    if ((config == NULL) || (strlen (config) == 0))
    {
        return INVALID_ARGUMENTS_ERROR;
    }
    if ((keep_alive) && ((config[0] == 'z') || (config[0] == 'Z')))
    {
        safe_logger (
            spdlog::level::err, "For Ganglion WIFI impedance works only if streaming is stopped");
        return INVALID_ARGUMENTS_ERROR;
    }
    if (config[0] == 'z')
    {
        is_cheking_impedance = true;
        return STATUS_OK;
    }
    if (config[0] == 'Z')
    {
        is_cheking_impedance = false;
        return STATUS_OK;
    }
    return send_config (config);
}

int GanglionWifi::start_stream (int buffer_size, char *streamer_params)
{
    if (keep_alive)
    {
        safe_logger (spdlog::level::err, "Streaming thread already running");
        return STREAM_ALREADY_RUN_ERROR;
    }
    if (buffer_size <= 0 || buffer_size > MAX_CAPTURE_SAMPLES)
    {
        safe_logger (spdlog::level::err, "invalid array size");
        return INVALID_BUFFER_SIZE_ERROR;
    }

    if (db)
    {
        delete db;
        db = NULL;
    }
    if (streamer)
    {
        delete streamer;
        streamer = NULL;
    }

    int res = prepare_streamer (streamer_params);
    if (res != STATUS_OK)
    {
        return res;
    }
    db = new DataBuffer (num_channels, buffer_size);
    if (!db->is_ready ())
    {
        safe_logger (spdlog::level::err, "unable to prepare buffer");
        delete db;
        db = NULL;
        return INVALID_BUFFER_SIZE_ERROR;
    }

    // need to start impedance streaming
    if (is_cheking_impedance)
    {
        res = send_config ("z");
        if (res == STATUS_OK)
        {
            keep_alive = true;
            streaming_thread = std::thread ([this] { this->read_thread_impedance (); });
            return STATUS_OK;
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
            return GENERAL_ERROR;
        }
        int send_res = wait_for_http_resp (request);
        if (send_res != STATUS_OK)
        {
            http_release (request);
            return send_res;
        }
        http_release (request);

        keep_alive = true;
        streaming_thread = std::thread ([this] { this->read_thread (); });
        return STATUS_OK;
    }
    return STATUS_OK;
}

int GanglionWifi::stop_stream ()
{
    if (keep_alive)
    {
        keep_alive = false;
        streaming_thread.join ();
        if (streamer)
        {
            delete streamer;
            streamer = NULL;
        }
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
            return GENERAL_ERROR;
        }
        int send_res = wait_for_http_resp (request);
        if (send_res != STATUS_OK)
        {
            http_release (request);
            return send_res;
        }
        http_release (request);

        return STATUS_OK;
    }
    else
    {
        return STREAM_THREAD_IS_NOT_RUNNING;
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
        if ((b[32] < END_BYTE_STANDARD) || (b[32] > END_BYTE_MAX))
        {
            safe_logger (spdlog::level::warn, "Wrong end byte, found {}", b[32]);
            continue;
        }

        // package num
        package[0] = (double)b[1];
        // eeg
        for (int i = 0; i < 4; i++)
        {
            package[i + 1] = eeg_scale * cast_24bit_to_int32 (b + 2 + 3 * i);
        }
        // end byte
        package[8] = (double)b[32];
        // place raw bytes to other_channels with end byte
        package[9] = (double)b[26];
        package[10] = (double)b[27];
        package[11] = (double)b[28];
        package[12] = (double)b[29];
        package[13] = (double)b[30];
        package[14] = (double)b[31];
        // place accel data
        if (b[32] == END_BYTE_STANDARD)
        {
            // accel
            // mistake in firmware in axis
            package[5] = accel_scale * cast_16bit_to_int32 (b + 28);
            package[6] = accel_scale * cast_16bit_to_int32 (b + 26);
            package[7] = -accel_scale * cast_16bit_to_int32 (b + 30);
        }
        // place analog data
        if (b[32] == END_BYTE_ANALOG)
        {
            // analog
            package[15] = cast_16bit_to_int32 (b + 26);
            package[16] = cast_16bit_to_int32 (b + 28);
            package[17] = cast_16bit_to_int32 (b + 30);
        }

        double timestamp = get_timestamp ();
        db->add_data (timestamp, package);
        streamer->stream_data (package, num_channels, timestamp);
    }
    delete[] package;
}

void GanglionWifi::read_thread_impedance ()
{
    int res;
    unsigned char b[OpenBCIWifiShieldBoard::package_size];
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

        double timestamp = get_timestamp ();
        db->add_data (timestamp, package);
        streamer->stream_data (package, num_channels, timestamp);
    }
    delete[] package;
}
