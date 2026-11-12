#include "expandable_eeg32_wifi.h"
#include "custom_cast.h"
#include "timestamp.h"

#ifndef _WIN32
#include <errno.h>
#endif

#define START_BYTE 0xA0
#define END_BYTE_STANDARD 0xC0
#define END_BYTE_ANALOG 0xC1
#define END_BYTE_MAX 0xC6

std::string ExpandableEeg32Wifi::translate_user_config_to_device (const std::string &conf)
{
    // Ch 1-16: native OpenBCI symbols. Ch 17-32: second-bank A-L / a-l per board spec.
    // Replace with a mapping table if your firmware uses different wire codes.
    return conf;
}

// load default settings for cyton boards
int ExpandableEeg32Wifi::prepare_session ()
{
    int res = OpenBCIWifiShieldBoard::prepare_session ();
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }
    return send_config ("d");
}

int ExpandableEeg32Wifi::config_board (std::string conf, std::string &response)
{
    std::string wire_config = translate_user_config_to_device (conf);
    if (gain_tracker.apply_config (wire_config) == (int)OpenBCICommandTypes::INVALID_COMMAND)
    {
        safe_logger (spdlog::level::warn, "invalid command: {}", conf.c_str ());
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    int res = OpenBCIWifiShieldBoard::config_board (wire_config, response);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        gain_tracker.revert_config ();
    }
    return res;
}

void ExpandableEeg32Wifi::read_thread ()
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
    unsigned char b[OpenBCIWifiShieldBoard::package_size];
    int num_rows = board_descr["default"]["num_rows"];
    double *package = new double[num_rows];
    for (int i = 0; i < num_rows; i++)
    {
        package[i] = 0.0;
    }
    bool first_sample = true;
    double accel[3] = {0.};
    unsigned char last_sample_id = 0;
    double accel_scale = (double)(0.002 / (pow (2, 4)));
    int sequence_count = 0;

    while (keep_alive)
    {
        res = server_socket->recv (b, OpenBCIWifiShieldBoard::package_size);
        if (res != OpenBCIWifiShieldBoard::package_size)
        {
            if (res < 0)
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
        unsigned char *bytes = b + 1; // for better consistency between plain cyton and wifi, in
                                      // plain cyton index is shifted by 1

        if ((bytes[31] < END_BYTE_STANDARD) || (bytes[31] > END_BYTE_MAX))
        {
            safe_logger (spdlog::level::warn, "Wrong end byte {}", bytes[31]);
            continue;
        }


        if (last_sample_id != bytes[0])
        {
            first_sample = true;
            sequence_count = 0;
        }
        last_sample_id = bytes[0];

        // place unprocessed bytes to other_channels for all modes
        // if (first_sample)
        // {
        package[0] = (double)bytes[0];
        // eeg
        for (int i = 0; i < 8; i++)
        {
            int gain_index = sequence_count * 8 + i;
            double eeg_scale = (double)(4.5 / float ((pow (2, 23) - 1)) /
                gain_tracker.get_gain_for_channel (gain_index) * 1000000.);
            package[i + 1 + sequence_count * 8] =
                eeg_scale * cast_24bit_to_int32 (bytes + 1 + 3 * i);
        }
        // other_channels
        // package[21] = (double)bytes[25];
        // package[22] = (double)bytes[26];
        // package[23] = (double)bytes[27];
        // package[24] = (double)bytes[28];
        // package[25] = (double)bytes[29];
        // package[26] = (double)bytes[30];
        // }
        // else
        // {
        //     // eeg
        //     for (int i = 0; i < 8; i++)
        //     {
        //         double eeg_scale = (double)(4.5 / float ((pow (2, 23) - 1)) /
        //             gain_tracker.get_gain_for_channel (i + 8) * 1000000.);
        //         package[i + 9] = eeg_scale * cast_24bit_to_int32 (bytes + 1 + 3 * i);
        //     }
        // need to average other_channels
        package[37] += (double)bytes[25];
        package[38] += (double)bytes[28];
        package[39] += (double)bytes[27];
        package[40] += (double)bytes[28];
        package[41] += (double)bytes[29];
        package[42] += (double)bytes[30];
        package[37] /= 2.0;
        package[38] /= 2.0;
        package[39] /= 2.0;
        package[40] /= 2.0;
        package[41] /= 2.0;
        package[42] /= 2.0;
        package[36] = (double)bytes[31];
        // }

        // place processed accel data
        if (bytes[31] == END_BYTE_STANDARD)
        {
            int32_t accel_temp[3] = {0};
            accel_temp[0] = cast_16bit_to_int32 (bytes + 25);
            accel_temp[1] = cast_16bit_to_int32 (bytes + 27);
            accel_temp[2] = cast_16bit_to_int32 (bytes + 29);

            if (first_sample)
            {
                package[0] = (double)bytes[0];

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

                package[36] = (double)bytes[31];
            }

            package[33] = accel[0];
            package[34] = accel[1];
            package[35] = accel[2];
        }
        // place processed analog data
        if (bytes[31] == END_BYTE_ANALOG)
        {
            if (first_sample)
            {
                package[0] = (double)bytes[0];
                // analog
                package[43] = cast_16bit_to_int32 (bytes + 25);
                package[44] = cast_16bit_to_int32 (bytes + 27);
                package[45] = cast_16bit_to_int32 (bytes + 29);
            }
            else
            {
                // need to average analog data
                package[43] += cast_16bit_to_int32 (bytes + 25);
                package[44] += cast_16bit_to_int32 (bytes + 27);
                package[45] += cast_16bit_to_int32 (bytes + 29);
                package[43] /= (float)(sequence_count + 1);
                package[44] /= (float)(sequence_count + 1);
                package[45] /= (float)(sequence_count + 1);
                package[36] = (double)bytes[31]; // cyton end byte
            }
        }
        // commit package (4 WiFi frames = 32 EEG channels for this board_id)
        sequence_count++;
        if (4 == sequence_count)
        {
            package[board_descr["default"]["timestamp_channel"].get<int> ()] = get_timestamp ();
            push_package (package);
            first_sample = false;
            // Must reset before next frame: otherwise same sample_id could yield gain_index 32..39
            // and debug STL asserts on current_gains[gain_index].
            sequence_count = 0;
        }
    }
    delete[] package;
}
