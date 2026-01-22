#include "ganglion_v2.h"

#include "custom_cast.h"

GanglionV2::GanglionV2 (struct BrainFlowInputParams params)
    : Ganglion (BoardIds::GANGLION_V2_BOARD, params)
{
}

void GanglionV2::decompress (
    struct GanglionLib::GanglionData *data, float *last_data, double *acceleration, double *package)
{
    int bits_per_num = 0;
    unsigned char package_bits[160] = {0}; // 20 * 8
    for (int i = 0; i < 20; i++)
    {
        uchar_to_bits (data->data[i], package_bits + i * 8);
    }

    float delta[8] = {0.f}; // delta holds 8 nums (4 by each package)

    // no compression, used to init variable
    if (data->data[0] == 0)
    {
        // shift the last data packet to make room for a newer one
        last_data[0] = last_data[4];
        last_data[1] = last_data[5];
        last_data[2] = last_data[6];
        last_data[3] = last_data[7];

        // add new packet
        last_data[4] = (float)cast_24bit_to_int32 (data->data + 1);
        last_data[5] = (float)cast_24bit_to_int32 (data->data + 4);
        last_data[6] = (float)cast_24bit_to_int32 (data->data + 7);
        last_data[7] = (float)cast_24bit_to_int32 (data->data + 10);

        // scale new packet and insert into result
        package[board_descr["default"]["package_num_channel"].get<int> ()] = 0.;
        package[board_descr["default"]["eeg_channels"][0].get<int> ()] = eeg_scale * last_data[4];
        package[board_descr["default"]["eeg_channels"][1].get<int> ()] = eeg_scale * last_data[5];
        package[board_descr["default"]["eeg_channels"][2].get<int> ()] = eeg_scale * last_data[6];
        package[board_descr["default"]["eeg_channels"][3].get<int> ()] = eeg_scale * last_data[7];
        package[board_descr["default"]["accel_channels"][0].get<int> ()] = acceleration[0];
        package[board_descr["default"]["accel_channels"][1].get<int> ()] = acceleration[1];
        package[board_descr["default"]["accel_channels"][2].get<int> ()] = acceleration[2];
        package[board_descr["default"]["timestamp_channel"].get<int> ()] = data->timestamp;
        push_package (package);
        return;
    }
    // 18 bit compression, sends delta from previous value instead of real value!
    else if (data->data[0] >= 1 && data->data[0] <= 100)
    {
        int last_digit = data->data[0] % 10;
        switch (last_digit)
        {
            // accel data is signed, so we must cast it to signed char
            // swap x and z, and invert z to convert to standard coordinate space.
            case 0:
                acceleration[2] = -accel_scale * (char)data->data[19];
                break;
            case 1:
                acceleration[1] = accel_scale * (char)data->data[19];
                break;
            case 2:
                acceleration[0] = accel_scale * (char)data->data[19];
                break;
            default:
                break;
        }
        bits_per_num = 18;
    }
    else if (data->data[0] >= 101 && data->data[0] <= 200)
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
        last_data[i] = last_data[i + 4] - delta[i];
    }

    // apply the second delta to the previous packet which we just decompressed above
    for (int i = 4; i < 8; i++)
    {
        last_data[i] = last_data[i - 4] - delta[i];
    }

    // add first encoded package
    package[board_descr["default"]["package_num_channel"].get<int> ()] = data->data[0];
    package[board_descr["default"]["eeg_channels"][0].get<int> ()] = eeg_scale * last_data[0];
    package[board_descr["default"]["eeg_channels"][1].get<int> ()] = eeg_scale * last_data[1];
    package[board_descr["default"]["eeg_channels"][2].get<int> ()] = eeg_scale * last_data[2];
    package[board_descr["default"]["eeg_channels"][3].get<int> ()] = eeg_scale * last_data[3];
    package[board_descr["default"]["accel_channels"][0].get<int> ()] = acceleration[0];
    package[board_descr["default"]["accel_channels"][1].get<int> ()] = acceleration[1];
    package[board_descr["default"]["accel_channels"][2].get<int> ()] = acceleration[2];
    package[board_descr["default"]["timestamp_channel"].get<int> ()] = data->timestamp;
    push_package (package);
    // add second package
    package[board_descr["default"]["eeg_channels"][0].get<int> ()] = eeg_scale * last_data[4];
    package[board_descr["default"]["eeg_channels"][1].get<int> ()] = eeg_scale * last_data[5];
    package[board_descr["default"]["eeg_channels"][2].get<int> ()] = eeg_scale * last_data[6];
    package[board_descr["default"]["eeg_channels"][3].get<int> ()] = eeg_scale * last_data[7];
    package[board_descr["default"]["timestamp_channel"].get<int> ()] = data->timestamp;
    push_package (package);
}