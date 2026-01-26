#include "ganglion_native_v2.h"

#include "custom_cast.h"
#include "timestamp.h"

GanglionNativeV2::GanglionNativeV2 (struct BrainFlowInputParams params)
    : GanglionNative (BoardIds::GANGLION_V2_NATIVE_BOARD, params)
{
}

void GanglionNativeV2::decompress (const uint8_t *data, double *package)
{
    int bits_per_num = 0;
    unsigned char package_bits[160] = {0}; // 20 * 8
    for (int i = 0; i < 20; i++)
    {
        uchar_to_bits (data[i], package_bits + i * 8);
    }

    float delta[8] = {0.f}; // delta holds 8 nums (4 by each package)

    // no compression, used to init variable
    if (data[0] == 0)
    {
        // shift the last data packet to make room for a newer one
        temp_data.last_data[0] = temp_data.last_data[4];
        temp_data.last_data[1] = temp_data.last_data[5];
        temp_data.last_data[2] = temp_data.last_data[6];
        temp_data.last_data[3] = temp_data.last_data[7];

        // add new packet
        temp_data.last_data[4] = (float)cast_24bit_to_int32 (data + 1);
        temp_data.last_data[5] = (float)cast_24bit_to_int32 (data + 4);
        temp_data.last_data[6] = (float)cast_24bit_to_int32 (data + 7);
        temp_data.last_data[7] = (float)cast_24bit_to_int32 (data + 10);

        // scale new packet and insert into result
        package[board_descr["default"]["package_num_channel"].get<int> ()] = 0.;
        package[board_descr["default"]["eeg_channels"][0].get<int> ()] =
            eeg_scale * temp_data.last_data[4];
        package[board_descr["default"]["eeg_channels"][1].get<int> ()] =
            eeg_scale * temp_data.last_data[5];
        package[board_descr["default"]["eeg_channels"][2].get<int> ()] =
            eeg_scale * temp_data.last_data[6];
        package[board_descr["default"]["eeg_channels"][3].get<int> ()] =
            eeg_scale * temp_data.last_data[7];
        package[board_descr["default"]["accel_channels"][0].get<int> ()] = temp_data.accel_x;
        package[board_descr["default"]["accel_channels"][1].get<int> ()] = temp_data.accel_y;
        package[board_descr["default"]["accel_channels"][2].get<int> ()] = temp_data.accel_z;
        package[board_descr["default"]["timestamp_channel"].get<int> ()] = get_timestamp ();
        push_package (package);
        return;
    }
    // 18 bit compression, sends delta from previous value instead of real value!
    if (data[0] >= 1 && data[0] <= 100)
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
    else if (data[0] >= 101 && data[0] <= 200)
    {
        bits_per_num = 19;
    }

    // handle compressed data for 18 or 19 bits
    for (int i = 8, counter = 0; i < bits_per_num * 8; i += bits_per_num, counter++)
    {
        if (bits_per_num == 18)
        {
            delta[counter] = (float)cast_ganglion_bits_to_int32<18> (package_bits + i);
        }
        else
        {
            delta[counter] = (float)cast_ganglion_bits_to_int32<19> (package_bits + i);
        }
    }

    // apply the first delta to the last data we got in the previous iteration
    for (int i = 0; i < 4; i++)
    {
        temp_data.last_data[i] = temp_data.last_data[i + 4] - delta[i];
    }

    // apply the second delta to the previous packet which we just decompressed above
    for (int i = 4; i < 8; i++)
    {
        temp_data.last_data[i] = temp_data.last_data[i - 4] - delta[i];
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