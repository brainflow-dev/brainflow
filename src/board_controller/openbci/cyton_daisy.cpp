#include <vector>

#include "custom_cast.h"
#include "cyton_daisy.h"
#include "serial.h"
#include "timestamp.h"

#define START_BYTE 0xA0
#define END_BYTE_STANDARD 0xC0
#define END_BYTE_ANALOG 0xC1
#define END_BYTE_MAX 0xC6


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
    bool first_sample = true;
    double accel[3] = {0.};
    double *package = new double[board_descr["num_rows"].get<int> ()];
    for (int i = 0; i < board_descr["num_rows"].get<int> (); i++)
    {
        package[i] = 0.0;
    }

    while (keep_alive)
    {
        // check start byte
        res = serial->read_from_serial_port (b, 1);
        if (res != 1)
        {
            safe_logger (spdlog::level::debug, "unable to read 1 byte");
            continue;
        }
        if (b[0] != START_BYTE)
        {
            continue;
        }

        int remaining_bytes = 32;
        int pos = 0;
        while ((remaining_bytes > 0) && (keep_alive))
        {
            res = serial->read_from_serial_port (b + pos, remaining_bytes);
            remaining_bytes -= res;
            pos += res;
        }
        if (!keep_alive)
        {
            break;
        }

        if ((b[31] < END_BYTE_STANDARD) || (b[31] > END_BYTE_MAX))
        {
            safe_logger (spdlog::level::warn, "Wrong end byte {}", b[31]);
            continue;
        }

        // For Cyton Daisy Serial, sample IDs are sequenctial
        // (0, 1, 2, 3...) so even sample IDs are the first sample (daisy)
        // and odd sample IDs are the second sample (cyton)
        // after the second sample, we commit the package below
        first_sample = b[0] % 2 == 0;

        // place unprocessed bytes to other_channels for all modes
        if (first_sample)
        {
            package[board_descr["package_num_channel"].get<int> ()] = (double)b[0];
            // eeg
            for (int i = 0; i < 8; i++)
            {
                package[i + 9] = eeg_scale * cast_24bit_to_int32 (b + 1 + 3 * i);
            }
            // other_channels
            package[21] = (double)b[25];
            package[22] = (double)b[26];
            package[23] = (double)b[27];
            package[24] = (double)b[28];
            package[25] = (double)b[29];
            package[26] = (double)b[30];
        }
        else
        {
            // eeg
            for (int i = 0; i < 8; i++)
            {
                package[i + 1] = eeg_scale * cast_24bit_to_int32 (b + 1 + 3 * i);
            }
            // need to average other_channels
            package[21] += (double)b[25];
            package[22] += (double)b[26];
            package[23] += (double)b[27];
            package[24] += (double)b[28];
            package[25] += (double)b[29];
            package[26] += (double)b[30];
            package[21] /= 2.0;
            package[22] /= 2.0;
            package[23] /= 2.0;
            package[24] /= 2.0;
            package[25] /= 2.0;
            package[26] /= 2.0;
            package[20] = (double)b[31];
        }

        // place processed accel data
        if (b[31] == END_BYTE_STANDARD)
        {
            int32_t accel_temp[3] = {0};
            accel_temp[0] = cast_16bit_to_int32 (b + 25);
            accel_temp[1] = cast_16bit_to_int32 (b + 27);
            accel_temp[2] = cast_16bit_to_int32 (b + 29);

            if (first_sample)
            {
                package[0] = (double)b[0];
                // accel
                if (accel_temp[0] != 0)
                {
                    accel[0] = accel_scale * accel_temp[0];
                    accel[1] = accel_scale * accel_temp[1];
                    accel[2] = accel_scale * accel_temp[2];
                }
            }
            else
            {
                // need to average accel data
                if (accel_temp[0] != 0)
                {
                    accel[0] += accel_scale * accel_temp[0];
                    accel[1] += accel_scale * accel_temp[1];
                    accel[2] += accel_scale * accel_temp[2];

                    accel[0] /= 2.f;
                    accel[1] /= 2.f;
                    accel[2] /= 2.f;
                }

                package[20] = (double)b[31];
            }

            package[17] = accel[0];
            package[18] = accel[1];
            package[19] = accel[2];
        }
        // place processed analog data
        if (b[31] == END_BYTE_ANALOG)
        {
            if (first_sample)
            {
                package[0] = (double)b[0];
                // analog
                package[27] = cast_16bit_to_int32 (b + 25);
                package[28] = cast_16bit_to_int32 (b + 27);
                package[29] = cast_16bit_to_int32 (b + 29);
            }
            else
            {
                // need to average analog data
                package[27] += cast_16bit_to_int32 (b + 25);
                package[28] += cast_16bit_to_int32 (b + 27);
                package[29] += cast_16bit_to_int32 (b + 29);
                package[27] /= 2.0f;
                package[28] /= 2.0f;
                package[29] /= 2.0f;
                package[20] = (double)b[31]; // cyton end byte
            }
        }
        // commit package
        if (!first_sample)
        {
            package[board_descr["timestamp_channel"].get<int> ()] = get_timestamp ();
            push_package (package);
        }
    }
    delete[] package;
}