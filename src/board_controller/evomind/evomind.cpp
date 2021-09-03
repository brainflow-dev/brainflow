#include <string.h>
#include <iomanip>

#include "evomind.h"

#include "custom_cast.h"
#include "get_dll_dir.h"
#include "timestamp.h"

#define START_BYTE 0xA0
#define END_BYTE_STANDARD 0xC0
#define END_BYTE_ANALOG 0xC1
#define END_BYTE_MAX 0xC6


Evomind::Evomind (struct BrainFlowInputParams params)
    : BTLibBoard ((int)BoardIds::EVOMIND_BOARD, params)
{
    keep_alive = false;
    state = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
}

Evomind::~Evomind ()
{
    skip_logs = true;
    release_session ();
}

int Evomind::start_stream (int buffer_size, char *streamer_params)
{
    if (!initialized)
    {
        safe_logger (spdlog::level::err, "You need to call prepare_session before config_board");
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    if (keep_alive)
    {
        safe_logger (spdlog::level::err, "Streaming thread already running");
        return (int)BrainFlowExitCodes::STREAM_ALREADY_RUN_ERROR;
    }

    int res = prepare_for_acquisition (buffer_size, streamer_params);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }

    res = bluetooth_open_device (1);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }

    keep_alive = true;
    streaming_thread = std::thread ([this] { this->read_thread (); });
    // wait for the 1st package received
    std::unique_lock<std::mutex> lk (this->m);
    auto sec = std::chrono::seconds (1);
    int num_secs = 5;
    if (cv.wait_for (lk, num_secs * sec,
            [this] { return this->state != (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR; }))
    {
        return state;
    }
    else
    {
        safe_logger (spdlog::level::err, "no data received in {} sec, stopping thread", num_secs);
        stop_stream ();
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
}

int Evomind::stop_stream ()
{
    if (keep_alive)
    {
        keep_alive = false;
        streaming_thread.join ();
        state = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
        return bluetooth_close_device ();
    }
    else
    {
        return (int)BrainFlowExitCodes::STREAM_THREAD_IS_NOT_RUNNING;
    }
}

int Evomind::release_session ()
{
    if (initialized)
    {
        stop_stream ();
        free_packages ();
    }
    return BTLibBoard::release_session ();
}

template<typename T>std::string int_to_hex( T i )
{
  std::stringstream stream;
  stream << "0x"
         << std::setfill ('0') << std::setw(sizeof(T)*2)
         << std::hex << i;
  return stream.str();
}

void Evomind::read_thread ()
{
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

    int num_rows = board_descr["num_rows"];
    double *package = new double[num_rows];
    for (int i = 0; i < num_rows; i++)
    {
        package[i] = 0.0;
    }
    constexpr int buf_size = 33;
    unsigned char temp_buffer[buf_size];
    for (int i = 0; i < buf_size; i++)
    {
        temp_buffer[i] = 0;
    }

    while (keep_alive)
    {
        bool is_ready = false;
        int res = bluetooth_get_data ((char *)temp_buffer, 1);
        safe_logger (spdlog::level::info, "CHECK START BYTE. res: {}, byte: {}", res, (int)temp_buffer[0]);
        if ((res == 1) && (temp_buffer[0] == START_BYTE))
        {
            is_ready = true;
        }
        else
        {
            continue;
        }

        double timestamp = get_timestamp ();
        // notify main thread that 1st byte received
        if (state != (int)BrainFlowExitCodes::STATUS_OK)
        {
            {
                std::lock_guard<std::mutex> lk (m);
                state = (int)BrainFlowExitCodes::STATUS_OK;
            }
            cv.notify_one ();
            safe_logger (spdlog::level::info, "start streaming");
        }

        if (is_ready)
        {
            while ((keep_alive) && (res >= 0))
            {
                for (int i = 0; i < buf_size; i++)
                {
                    temp_buffer[i] = 0;
                }
                res = bluetooth_get_data ((char *)temp_buffer + 1, buf_size - 1);
                if (res == 0) {
                    sleep (5);
                }
                safe_logger (spdlog::level::info, "DATA BYTES. res: {}", res);
                for (int i = 0; i < buf_size; i++) {
                    safe_logger (spdlog::level::info, "data byte #{} = {}", i, temp_buffer[i]);
                }
                if (res == buf_size - 1)
                {
                    std::vector<int> eeg_channels = board_descr["eeg_channels"];
                    for (int i = 0; i <= 9; i++)
                    {
                        int32_t val = 0;
                        memcpy (&val, temp_buffer + i * sizeof (int32_t), sizeof (int32_t));
                        val = swap_endians (val);
                        if (i == 0)
                        {
                            package[board_descr["package_num_channel"].get<int> ()] = val % 65536;
                        }
                        else
                        {
                            double value = ((double)(val / 256)) * 5.0 / 8388608.0;
                            double gain = 100.0;
                            value = value / gain * 1000000.0;
                            package[eeg_channels[i - 1]] = value;
                        }
                    }
                    package[board_descr["timestamp_channel"].get<int> ()] = timestamp;
                    push_package (package);
                    break;
                }
            }
        }

//        int remaining_bytes = 32;
//        int pos = 0;
//        while ((remaining_bytes > 0) && (keep_alive))
//        {
//            res = bluetooth_get_data ((char *)(temp_buffer + pos), remaining_bytes);
//            if (res == 1)
//                safe_logger (spdlog::level::info, "read new data byte: {}", res);
//            remaining_bytes -= res;
//            pos += res;
//        }
//        if (!keep_alive)
//        {
//            break;
//        }
//
//        if ((temp_buffer[31] < END_BYTE_STANDARD) || (temp_buffer[31] > END_BYTE_MAX))
//        {
//            safe_logger (spdlog::level::warn, "Wrong end byte {}", temp_buffer[31]);
//            continue;
//        }
//
//        // package num
//        package[board_descr["package_num_channel"].get<int> ()] = (double)temp_buffer[0];
//        // eeg
//        for (unsigned int i = 0; i < eeg_channels.size (); i++)
//        {
//            package[eeg_channels[i]] = eeg_scale * cast_24bit_to_int32 (temp_buffer + 1 + 3 * i);
//        }
//
//        package[board_descr["timestamp_channel"].get<int> ()] = get_timestamp ();
//        push_package (package);
    }
    delete[] package;
}

int Evomind::config_board (std::string config, std::string &response)
{
    safe_logger (spdlog::level::debug, "config_board is not supported for Evomind");
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

std::string Evomind::get_name_selector ()
{
    return "nophone";
}