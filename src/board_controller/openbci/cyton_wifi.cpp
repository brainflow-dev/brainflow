#include <vector>

#include "custom_cast.h"
#include "cyton_wifi.h"
#include "timestamp.h"

#ifndef _WIN32
#include <errno.h>
#endif


#define START_BYTE 0xA0
#define END_BYTE_STANDARD 0xC0
#define END_BYTE_ANALOG 0xC1
#define END_BYTE_MAX 0xC6


// load default settings for cyton boards
int CytonWifi::prepare_session ()
{
    int res = OpenBCIWifiShieldBoard::prepare_session ();
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }
    return send_config ("d");
}

void CytonWifi::read_thread ()
{
    /*
        Byte 1: 0xA0
        Byte 2: Sample Number
        Bytes 3-5: Data value for EEG channel 1
        Bytes 6-8: Data value for EEG channel 2
        Bytes 9-11: Data value for EEG channel 3
        Bytes 12-14: Data value for EEG channel 4
        Bytes 15-17: Data value for EEG channel 5
        Bytes 18-20: Data value for EEG channel 6
        Bytes 21-23: Data value for EEG channel 6
        Bytes 24-26: Data value for EEG channel 8
        Aux Data Bytes 27-32: 6 bytes of data
        Byte 33: 0xCX where X is 0-F in hex
    */
    int res;
    unsigned char b[OpenBCIWifiShieldBoard::package_size];
    double accel[3] = {0.};
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
        unsigned char *bytes = b + 1; // for better consistency between plain cyton and wifi, in
                                      // plain cyton index is shifted by 1

        if ((bytes[31] < END_BYTE_STANDARD) || (bytes[31] > END_BYTE_MAX))
        {
            safe_logger (spdlog::level::warn, "Wrong end byte {}", bytes[31]);
            continue;
        }

        // package num
        package[board_descr["package_num_channel"].get<int> ()] = (double)bytes[0];
        // eeg
        for (unsigned int i = 0; i < eeg_channels.size (); i++)
        {
            package[eeg_channels[i]] = eeg_scale * cast_24bit_to_int32 (bytes + 1 + 3 * i);
        }
        package[board_descr["other_channels"][0].get<int> ()] = (double)bytes[31]; // end byte
        // place unprocessed bytes for all modes to other_channels
        package[board_descr["other_channels"][1].get<int> ()] = (double)bytes[25];
        package[board_descr["other_channels"][2].get<int> ()] = (double)bytes[26];
        package[board_descr["other_channels"][3].get<int> ()] = (double)bytes[27];
        package[board_descr["other_channels"][4].get<int> ()] = (double)bytes[28];
        package[board_descr["other_channels"][5].get<int> ()] = (double)bytes[29];
        package[board_descr["other_channels"][6].get<int> ()] = (double)bytes[30];
        // place processed bytes for accel
        if (bytes[31] == END_BYTE_STANDARD)
        {
            int32_t accel_temp[3] = {0};
            accel_temp[0] = cast_16bit_to_int32 (bytes + 25);
            accel_temp[1] = cast_16bit_to_int32 (bytes + 27);
            accel_temp[2] = cast_16bit_to_int32 (bytes + 29);

            if (accel_temp[0] != 0)
            {
                accel[0] = accel_scale * accel_temp[0];
                accel[1] = accel_scale * accel_temp[1];
                accel[2] = accel_scale * accel_temp[2];
            }

            package[board_descr["accel_channels"][0].get<int> ()] = accel[0];
            package[board_descr["accel_channels"][1].get<int> ()] = accel[1];
            package[board_descr["accel_channels"][2].get<int> ()] = accel[2];
        }
        // place processed bytes for analog
        if (bytes[31] == END_BYTE_ANALOG)
        {
            package[board_descr["analog_channels"][0].get<int> ()] =
                cast_16bit_to_int32 (bytes + 25);
            package[board_descr["analog_channels"][1].get<int> ()] =
                cast_16bit_to_int32 (bytes + 27);
            package[board_descr["analog_channels"][2].get<int> ()] =
                cast_16bit_to_int32 (bytes + 29);
        }

        package[board_descr["timestamp_channel"].get<int> ()] = get_timestamp ();
        push_package (package);
    }
    delete[] package;
}
