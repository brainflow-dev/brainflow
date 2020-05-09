#include "ganglion_wifi.h"
#include "custom_cast.h"
#include "timestamp.h"

#ifndef _WIN32
#include <errno.h>
#endif

#define START_BYTE 0xA0
#define END_BYTE_STANDARD 0xC0
#define END_BYTE_ANALOG 0xC1
#define END_BYTE_MAX 0xC6


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

        double package[18] = {0.};
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
        streamer->stream_data (package, 18, timestamp);
    }
}
