#include "ganglion_native_v3.h"

#include "custom_cast.h"
#include "timestamp.h"

GanglionNativeV3::GanglionNativeV3 (struct BrainFlowInputParams params)
    : GanglionNative (BoardIds::GANGLION_NATIVE_BOARD, params)
{
}

void GanglionNativeV3::decompress (const uint8_t *data, double *package)
{
    int bits_per_num = 0;
    unsigned char package_bits[160] = {0}; // 20 * 8
    for (int i = 0; i < 20; i++)
    {
        uchar_to_bits (data[i], package_bits + i * 8);
    }

    // 18 bit compression, sends 17 MSBs + sign bit of 24-bit sample
    if (data[0] >= 0 && data[0] < 100)
    {
        int last_digit = data[0] % 10;
        switch (last_digit)
        {
            // accel data is signed, so we must cast it to signed char
            // swap x and z, and invert z to convert to standard coordinate space.
            case 0:
                temp_data.accel_z = -accel_scale * (char)data[19];
                break;
            case 1:
                temp_data.accel_y = accel_scale * (char)data[19];
                break;
            case 2:
                temp_data.accel_x = accel_scale * (char)data[19];
                break;
            default:
                break;
        }
        bits_per_num = 18;
    }
    else if (data[0] >= 100 && data[0] < 200)
    {
        bits_per_num = 19;
    }

    // handle compressed data for 18 or 19 bits
    for (int i = 8, counter = 0; i < bits_per_num * 8; i += bits_per_num, counter++)
    {
        if (bits_per_num == 18)
        {
            temp_data.last_data[counter] =
                (float)(cast_ganglion_bits_to_int32<18> (package_bits + i) << 6);
        }
        else
        {
            temp_data.last_data[counter] =
                (float)(cast_ganglion_bits_to_int32<19> (package_bits + i) << 5);
        }
    }

    // add first encoded package
    package[board_descr["default"]["package_num_channel"].get<int> ()] = data[0];
    package[board_descr["default"]["eeg_channels"][0].get<int> ()] =
        eeg_scale * temp_data.last_data[0];
    package[board_descr["default"]["eeg_channels"][1].get<int> ()] =
        eeg_scale * temp_data.last_data[1];
    package[board_descr["default"]["eeg_channels"][2].get<int> ()] =
        eeg_scale * temp_data.last_data[2];
    package[board_descr["default"]["eeg_channels"][3].get<int> ()] =
        eeg_scale * temp_data.last_data[3];
    package[board_descr["default"]["accel_channels"][0].get<int> ()] = temp_data.accel_x;
    package[board_descr["default"]["accel_channels"][1].get<int> ()] = temp_data.accel_y;
    package[board_descr["default"]["accel_channels"][2].get<int> ()] = temp_data.accel_z;
    package[board_descr["default"]["timestamp_channel"].get<int> ()] = get_timestamp ();
    push_package (package);
    // add second package
    package[board_descr["default"]["eeg_channels"][0].get<int> ()] =
        eeg_scale * temp_data.last_data[4];
    package[board_descr["default"]["eeg_channels"][1].get<int> ()] =
        eeg_scale * temp_data.last_data[5];
    package[board_descr["default"]["eeg_channels"][2].get<int> ()] =
        eeg_scale * temp_data.last_data[6];
    package[board_descr["default"]["eeg_channels"][3].get<int> ()] =
        eeg_scale * temp_data.last_data[7];
    package[board_descr["default"]["timestamp_channel"].get<int> ()] = get_timestamp ();
    push_package (package);
}