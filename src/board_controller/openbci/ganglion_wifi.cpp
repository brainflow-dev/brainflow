#include "ganglion_wifi.h"
#include "custom_cast.h"
#include "timestamp.h"


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
    unsigned char b[OpenBCIWifiShieldBoard::transaction_size];
    while (keep_alive)
    {
        // check start byte
        res = server_socket->recv (b, OpenBCIWifiShieldBoard::transaction_size);
        if (res != OpenBCIWifiShieldBoard::transaction_size)
        {
            continue;
        }
        for (int cur_package = 0;
             cur_package < OpenBCIWifiShieldBoard::num_packages_per_transaction; cur_package++)
        {
            int offset = cur_package * OpenBCIWifiShieldBoard::package_size;
            if (b[0 + offset] != START_BYTE)
            {
                continue;
            }

            double package[18] = {0.};
            // package num
            package[0] = (double)b[1 + offset];
            // eeg
            for (int i = 0; i < 4; i++)
            {
                package[i + 1] = eeg_scale * cast_24bit_to_int32 (b + 2 + 3 * i + offset);
            }
            // end byte
            package[8] = (double)b[32 + offset];
            // check end byte
            if (b[32 + offset] == END_BYTE_STANDARD)
            {
                // accel
                package[5] = accel_scale * cast_16bit_to_int32 (b + 26 + offset);
                package[6] = accel_scale * cast_16bit_to_int32 (b + 28 + offset);
                package[7] = accel_scale * cast_16bit_to_int32 (b + 30 + offset);
            }
            else if (b[32 + offset] == END_BYTE_ANALOG)
            {
                // analog
                package[15] = cast_16bit_to_int32 (b + 26 + offset);
                package[16] = cast_16bit_to_int32 (b + 28 + offset);
                package[17] = cast_16bit_to_int32 (b + 30 + offset);
            }
            else if ((b[32 + offset] > END_BYTE_ANALOG) && (b[32 + offset] <= END_BYTE_MAX))
            {
                // other data
                package[8] = (double)b[32 + offset];
                package[9] = (double)b[26 + offset];
                package[10] = (double)b[27 + offset];
                package[11] = (double)b[28 + offset];
                package[12] = (double)b[29 + offset];
                package[13] = (double)b[30 + offset];
                package[14] = (double)b[31 + offset];
            }
            else
            {
                safe_logger (spdlog::level::warn, "Wrong end byte, found {}", b[32 + offset]);
                continue;
            }

            double timestamp = get_timestamp ();
            db->add_data (timestamp, package);
            streamer->stream_data (package, 18, timestamp);
        }
    }
}
