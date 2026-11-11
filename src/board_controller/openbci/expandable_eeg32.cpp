#include <vector>

#include "custom_cast.h"
#include "expandable_eeg32.h"
#include "serial.h"
#include "timestamp.h"

#define START_BYTE 0xA0
#define END_BYTE_STANDARD 0xC0
#define END_BYTE_ANALOG 0xC1
#define END_BYTE_MAX 0xC6
#define NUM_BANK_PACKETS 4


int ExpandableEeg32::config_board (std::string conf, std::string &response)
{
    if (gain_tracker.apply_config (conf) == (int)OpenBCICommandTypes::INVALID_COMMAND)
    {
        safe_logger (spdlog::level::warn, "invalid command: {}", conf.c_str ());
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    int res = OpenBCISerialBoard::config_board (conf, response);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        gain_tracker.revert_config ();
    }
    return res;
}

void ExpandableEeg32::read_thread ()
{
    // 4 consecutive 33-byte Cyton packets -> one 32-ch BrainFlow row (same layout as WiFi).
    // Firmware may send either:
    //   A) same sample_id x4 (like WiFi 32ch / SD 32ch), or
    //   B) sequential ids (0,1,2,3,4,5,6,7,...) as a Daisy-style extension.
    // Resync: if the next id is neither "same as first" nor "first+seq", start a new group.
    int res;
    unsigned char b[32];
    double accel[3] = {0.};
    int analog[3] = {0};
    int analog_count = 0;
    int accel_count = 0;
    int aux_count = 0;
    int sequence_count = 0;
    unsigned char raw_ids[NUM_BANK_PACKETS] = {0};
    unsigned char raw_ends[NUM_BANK_PACKETS] = {0};
    int num_rows = board_descr["default"]["num_rows"];
    double *package = new double[num_rows];
    for (int i = 0; i < num_rows; i++)
    {
        package[i] = 0.0;
    }
    double accel_scale = (double)(0.002 / (pow (2, 4)));

    while (keep_alive)
    {
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

        unsigned char sample_id = b[0];
        if (sequence_count > 0)
        {
            unsigned char first_id = raw_ids[0];
            unsigned char expect_same = first_id;
            unsigned char expect_seq = (unsigned char)((first_id + sequence_count) & 0xFF);
            if ((sample_id != expect_same) && (sample_id != expect_seq))
            {
                safe_logger (spdlog::level::debug,
                    "32ch serial resync: group_len={} first_id={} got={}", sequence_count,
                    (int)first_id, (int)sample_id);
                sequence_count = 0;
            }
        }

        if (sequence_count == 0)
        {
            for (int i = 0; i < num_rows; i++)
            {
                package[i] = 0.0;
            }
            package[board_descr["default"]["package_num_channel"].get<int> ()] = (double)sample_id;
            aux_count = 0;
            analog_count = 0;
            accel_count = 0;
            analog[0] = analog[1] = analog[2] = 0;
            accel[0] = accel[1] = accel[2] = 0.;
        }

        int bank = sequence_count;
        raw_ids[bank] = sample_id;
        raw_ends[bank] = b[31];

        for (int i = 0; i < 8; i++)
        {
            int gain_index = bank * 8 + i;
            double eeg_scale = (double)(4.5 / float ((pow (2, 23) - 1)) /
                gain_tracker.get_gain_for_channel (gain_index) * 1000000.);
            package[i + 1 + bank * 8] = eeg_scale * cast_24bit_to_int32 (b + 1 + 3 * i);
        }

        package[37] += (double)b[25];
        package[38] += (double)b[26];
        package[39] += (double)b[27];
        package[40] += (double)b[28];
        package[41] += (double)b[29];
        package[42] += (double)b[30];
        aux_count++;

        if (b[31] == END_BYTE_STANDARD)
        {
            int32_t accel_temp[3] = {0};
            accel_temp[0] = cast_16bit_to_int32 (b + 25);
            accel_temp[1] = cast_16bit_to_int32 (b + 27);
            accel_temp[2] = cast_16bit_to_int32 (b + 29);
            if (accel_temp[0] != 0)
            {
                if (accel_count == 0)
                {
                    accel[0] = accel_scale * accel_temp[0];
                    accel[1] = accel_scale * accel_temp[1];
                    accel[2] = accel_scale * accel_temp[2];
                }
                else
                {
                    accel[0] += accel_scale * accel_temp[0];
                    accel[1] += accel_scale * accel_temp[1];
                    accel[2] += accel_scale * accel_temp[2];
                }
                accel_count++;
            }
            package[33] = (accel_count > 0) ? (accel[0] / accel_count) : accel[0];
            package[34] = (accel_count > 0) ? (accel[1] / accel_count) : accel[1];
            package[35] = (accel_count > 0) ? (accel[2] / accel_count) : accel[2];
        }

        if (b[31] == END_BYTE_ANALOG)
        {
            analog[0] += cast_16bit_to_int32 (b + 25);
            analog[1] += cast_16bit_to_int32 (b + 27);
            analog[2] += cast_16bit_to_int32 (b + 29);
            analog_count++;
            package[43] = (analog_count > 0) ? (analog[0] / (double)analog_count) : 0.0;
            package[44] = (analog_count > 0) ? (analog[1] / (double)analog_count) : 0.0;
            package[45] = (analog_count > 0) ? (analog[2] / (double)analog_count) : 0.0;
        }

        sequence_count++;
        if (sequence_count == NUM_BANK_PACKETS)
        {
            if (aux_count > 0)
            {
                package[37] /= (double)aux_count;
                package[38] /= (double)aux_count;
                package[39] /= (double)aux_count;
                package[40] /= (double)aux_count;
                package[41] /= (double)aux_count;
                package[42] /= (double)aux_count;
            }
            // Overlay 4 raw sample_ids into other_channels 37-40 for the test tool.
            // Aux averages remain in 41-42 (partial). End byte of last sub-packet -> 36.
            package[36] = (double)raw_ends[3];
            package[37] = (double)raw_ids[0];
            package[38] = (double)raw_ids[1];
            package[39] = (double)raw_ids[2];
            package[40] = (double)raw_ids[3];
            package[board_descr["default"]["timestamp_channel"].get<int> ()] = get_timestamp ();
            push_package (package);
            sequence_count = 0;
        }
    }
    delete[] package;
}
