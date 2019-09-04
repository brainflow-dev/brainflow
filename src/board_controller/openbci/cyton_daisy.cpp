#include "cyton_daisy.h"
#include "custom_cast.h"
#include "serial.h"
#include "timestamp.h"

#define START_BYTE 0xA0
#define END_BYTE 0xC0


void CytonDaisy::read_thread ()
{
    // format is the same as for cyton but need to join two packages together
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
    unsigned char b[32];
    float package[20]; // 16 eeg channelsm 3 accel and package num
    bool first_sample = false;
    while (keep_alive)
    {
        // check start byte
        res = serial.read_from_serial_port (b, 1);
        if (res != 1)
        {
            safe_logger (spdlog::level::debug, "unable to read 1 byte");
            continue;
        }
        if (b[0] != START_BYTE)
        {
            continue;
        }

        res = serial.read_from_serial_port (b, 32);
        if (res != 32)
        {
            safe_logger (spdlog::level::debug, "unable to read 32 bytes");
            continue;
        }
        // check end byte
        if (b[res - 1] != END_BYTE)
        {
            safe_logger (
                spdlog::level::warn, "Wrong end byte, found {}, required {}", b[res - 1], END_BYTE);
            continue;
        }

        if ((b[0] % 2 == 0) && (first_sample))
        {
            // eeg
            for (int i = 0; i < 8; i++)
            {
                package[i + 9] = eeg_scale * cast_24bit_to_int32 (b + 1 + 3 * i);
            }
            // need to average accel data
            package[17] += accel_scale * cast_16bit_to_int32 (b + 25);
            package[18] += accel_scale * cast_16bit_to_int32 (b + 27);
            package[19] += accel_scale * cast_16bit_to_int32 (b + 29);
            package[17] /= 2.0f;
            package[18] /= 2.0f;
            package[19] /= 2.0f;
            db->add_data (get_timestamp (), package);
        }
        else
        {
            first_sample = true;
            package[0] = (float)b[0];
            // eeg
            for (int i = 0; i < 8; i++)
            {
                package[i + 1] = eeg_scale * cast_24bit_to_int32 (b + 1 + 3 * i);
            }
            // accel
            package[17] = accel_scale * cast_16bit_to_int32 (b + 25);
            package[18] = accel_scale * cast_16bit_to_int32 (b + 27);
            package[19] = accel_scale * cast_16bit_to_int32 (b + 29);
        }
    }
}