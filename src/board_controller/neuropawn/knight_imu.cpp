#include <cstdint>
#include <cstring>
#include <limits>
#include <math.h>
#include <vector>

#include "custom_cast.h"
#include "knight_imu.h"
#include "timestamp.h"

KnightIMU::KnightIMU (int board_id, struct BrainFlowInputParams params)
    : KnightBase (board_id, params)
{
}

void KnightIMU::read_thread ()
{
    /*
    Frame format (Arduino):
    [0]    1 Byte : START (0xA0)
    [1]    1 Byte : counter
    [2..17]16 Bytes: 8x EXG int16 (little-endian)
    [18]   1 Byte : LOFF STATP
    [19]   1 Byte : LOFF STATN
    [20..55]36 Bytes: 9x float32 IMU, little-endian: ax,ay,az,gx,gy,gz,mx,my,mz
    [56]   1 Byte : END (0xC0)
    */

    int res;
    constexpr int exg_channels_count = 8;
    constexpr int imu_channels_count = 9;
    constexpr int loff_bytes = 2;
    constexpr int frame_payload_size = 1 /*counter*/ + (exg_channels_count * 2) + loff_bytes +
        (imu_channels_count * 4) + 1 /*end*/;

    unsigned char b[frame_payload_size] = {0};

    float eeg_scale = 4 / float ((pow (2, 15) - 1)) / 12 * 1000000.;
    int num_rows = board_descr["default"]["num_rows"];
    double *package = new double[num_rows];
    for (int i = 0; i < num_rows; i++)
    {
        package[i] = 0.0;
    }

    std::vector<int> eeg_channels = board_descr["default"]["eeg_channels"];
    std::vector<int> other_channels = board_descr["default"]["other_channels"];

    while (keep_alive)
    {
        // checking the start byte
        unsigned char start = 0;
        res = serial->read_from_serial_port (&start, 1);
        if (res != 1)
        {
            safe_logger (spdlog::level::debug, "unable to read 1 byte, {}");
            continue;
        }
        if (start != KnightBase::start_byte)
        {
            continue;
        }

        int remaining_bytes = frame_payload_size;
        int pos = 0;
        while ((remaining_bytes > 0) && (keep_alive))
        {
            res = serial->read_from_serial_port (b + pos, remaining_bytes);
            if (res > 0)
            {
                remaining_bytes -= res;
                pos += res;
            }
        }

        if (!keep_alive)
        {
            break;
        }

        if (b[frame_payload_size - 1] != KnightBase::end_byte)
        {
            safe_logger (spdlog::level::warn, "Wrong end byte {}", b[frame_payload_size - 1]);
            continue;
        }

        // package number / counter
        package[board_descr["default"]["package_num_channel"].get<int> ()] = (double)b[0];

        // exg data retrieval
        const int exg_offset = 1;
        for (unsigned int i = 0; i < eeg_channels.size () && i < exg_channels_count; i++)
        {
            package[eeg_channels[i]] = eeg_scale * cast_16bit_to_int32 (b + exg_offset + 2 * i);
        }

        // other channel data retrieval (keep old behavior)
        const int loff_offset = exg_offset + (exg_channels_count * 2);
        package[other_channels[0]] = (double)b[loff_offset];     // LOFF STATP
        package[other_channels[1]] = (double)b[loff_offset + 1]; // LOFF STATN

        // IMU float32, little-endian
        const int imu_offset = loff_offset + loff_bytes;
        for (int i = 0; i < imu_channels_count && (2 + i) < (int)other_channels.size (); i++)
        {
            uint32_t u = (uint32_t)b[imu_offset + 4 * i] |
                ((uint32_t)b[imu_offset + 4 * i + 1] << 8) |
                ((uint32_t)b[imu_offset + 4 * i + 2] << 16) |
                ((uint32_t)b[imu_offset + 4 * i + 3] << 24);
            float f = 0.0f;
            static_assert (sizeof (float) == 4, "float must be 4 bytes");
            std::memcpy (&f, &u, sizeof (f));
            package[other_channels[2 + i]] = (double)f;
        }

        // time stamp channel
        package[board_descr["default"]["timestamp_channel"].get<int> ()] = get_timestamp ();

        push_package (package);
    }
    delete[] package;
}