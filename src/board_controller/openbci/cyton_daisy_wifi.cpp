#include "cyton_daisy_wifi.h"
#include "custom_cast.h"
#include "timestamp.h"


#define START_BYTE 0xA0
#define END_BYTE_STANDARD 0xC0
#define END_BYTE_ANALOG 0xC1
#define END_BYTE_MAX 0xC6


void CytonDaisyWifi::read_thread ()
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
    unsigned char b[OpenBCIWifiShieldBoard::transaction_size];
    while (keep_alive)
    {
        // check start byte
        res = server_socket->recv (b, OpenBCIWifiShieldBoard::transaction_size);
        if (res != OpenBCIWifiShieldBoard::transaction_size)
        {
            continue;
        }
        double package[30] = {0.};
        bool first_sample = false;
        for (int cur_package = 0;
             cur_package < OpenBCIWifiShieldBoard::num_packages_per_transaction; cur_package++)
        {
            int offset = cur_package * OpenBCIWifiShieldBoard::package_size;
            if (b[0 + offset] != START_BYTE)
            {
                break; // drop entire transaction for daisy
            }

            // check end byte
            if (b[32 + offset] == END_BYTE_STANDARD) // package has accel data
            {
                if ((b[1 + offset] % 2 == 0) && (first_sample))
                {
                    // eeg
                    for (int i = 0; i < 8; i++)
                    {
                        package[i + 9] = eeg_scale * cast_24bit_to_int32 (b + 2 + 3 * i + offset);
                    }
                    // need to average accel data
                    package[17] += accel_scale * cast_16bit_to_int32 (b + 26 + offset);
                    package[18] += accel_scale * cast_16bit_to_int32 (b + 28 + offset);
                    package[19] += accel_scale * cast_16bit_to_int32 (b + 30 + offset);
                    package[17] /= 2.0f;
                    package[18] /= 2.0f;
                    package[19] /= 2.0f;
                    package[20] = (double)b[32 + offset];
                    db->add_data (get_timestamp (), package);
                }
                else
                {
                    first_sample = true;
                    package[0] = (double)b[1 + offset];
                    // eeg
                    for (int i = 0; i < 8; i++)
                    {
                        package[i + 1] = eeg_scale * cast_24bit_to_int32 (b + 2 + 3 * i + offset);
                    }
                    // accel
                    package[17] = accel_scale * cast_16bit_to_int32 (b + 26 + offset);
                    package[18] = accel_scale * cast_16bit_to_int32 (b + 28 + offset);
                    package[19] = accel_scale * cast_16bit_to_int32 (b + 30 + offset);
                }
            }
            else if (b[32 + offset] == END_BYTE_ANALOG) // package has analog data
            {
                if ((b[1 + offset] % 2 == 0) && (first_sample))
                {
                    // eeg
                    for (int i = 0; i < 8; i++)
                    {
                        package[i + 9] = eeg_scale * cast_24bit_to_int32 (b + 2 + 3 * i + offset);
                    }
                    // need to average analog data
                    package[27] += cast_16bit_to_int32 (b + 26 + offset);
                    package[28] += cast_16bit_to_int32 (b + 28 + offset);
                    package[29] += cast_16bit_to_int32 (b + 30 + offset);
                    package[27] /= 2.0f;
                    package[28] /= 2.0f;
                    package[29] /= 2.0f;
                    package[20] = (double)b[32 + offset]; // cyton end byte
                    db->add_data (get_timestamp (), package);
                }
                else
                {
                    first_sample = true;
                    package[0] = (double)b[0 + offset];
                    // eeg
                    for (int i = 0; i < 8; i++)
                    {
                        package[i + 1] = eeg_scale * cast_24bit_to_int32 (b + 2 + 3 * i + offset);
                    }
                    // analog
                    package[27] = cast_16bit_to_int32 (b + 26 + offset);
                    package[28] = cast_16bit_to_int32 (b + 28 + offset);
                    package[29] = cast_16bit_to_int32 (b + 30 + offset);
                }
            }
            else if ((b[32 + offset] > END_BYTE_STANDARD) &&
                (b[32 + offset] <=
                    END_BYTE_MAX)) // some data which we dont preprocess but add as raw
            {
                if ((b[1 + offset] % 2 == 0) && (first_sample))
                {
                    // eeg
                    for (int i = 0; i < 8; i++)
                    {
                        package[i + 9] = eeg_scale * cast_24bit_to_int32 (b + 2 + 3 * i + offset);
                    }
                    // need to average other data
                    package[21] += (double)b[26 + offset];
                    package[22] += (double)b[27 + offset];
                    package[23] += (double)b[28 + offset];
                    package[24] += (double)b[29 + offset];
                    package[25] += (double)b[30 + offset];
                    package[26] += (double)b[31 + offset];
                    package[21] /= 2.0;
                    package[22] /= 2.0;
                    package[23] /= 2.0;
                    package[24] /= 2.0;
                    package[25] /= 2.0;
                    package[26] /= 2.0;
                    package[20] = (double)b[32 + offset];
                    db->add_data (get_timestamp (), package);
                }
                else
                {
                    first_sample = true;
                    package[0] = (double)b[1 + offset];
                    // eeg
                    for (int i = 0; i < 8; i++)
                    {
                        package[i + 1] = eeg_scale * cast_24bit_to_int32 (b + 2 + 3 * i + offset);
                    }
                    // other data
                    package[21] = (double)b[26 + offset];
                    package[22] = (double)b[27 + offset];
                    package[23] = (double)b[28 + offset];
                    package[24] = (double)b[29 + offset];
                    package[25] = (double)b[30 + offset];
                    package[26] = (double)b[31 + offset];
                }
            }
            else
            {
                safe_logger (spdlog::level::warn, "Wrong end byte, found {}", b[32 + offset]);
                break; // drop entire transaction
            }
        }
    }
}