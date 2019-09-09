#include <chrono>
#include <string.h>

#include "custom_cast.h"
#include "novaxr.h"
#include "openbci_helpers.h"

NovaXR::NovaXR (char *ip_addr) : Board (), socket (ip_addr, 2390, (int)SocketType::UDP)
{
    this->is_streaming = false;
    this->keep_alive = false;
    this->initialized = false;
    this->num_channels = 23;
    this->state = SYNC_TIMEOUT_ERROR;
}

NovaXR::~NovaXR ()
{
    skip_logs = true;
    release_session ();
}

int NovaXR::prepare_session ()
{
    if (initialized)
    {
        safe_logger (spdlog::level::info, "Session is already prepared");
        return STATUS_OK;
    }
    int res = socket.connect ();
    if (res != 0)
    {
        safe_logger (spdlog::level::err, "failed to init socket: {}", res);
        return GENERAL_ERROR;
    }
    initialized = true;
    return STATUS_OK;
}

int NovaXR::config_board (char *config)
{
    safe_logger (spdlog::level::debug, "Trying to config NovaXR with {}", config);
    int res = validate_config (config);
    if (res != STATUS_OK)
    {
        return res;
    }
    int len = strlen (config);
    res = socket.send (config, len);
    if (len != res)
    {
        safe_logger (spdlog::level::err, "Failed to config a board");
        return BOARD_WRITE_ERROR;
    }
    return STATUS_OK;
}

int NovaXR::start_stream (int buffer_size)
{
    if (is_streaming)
    {
        safe_logger (spdlog::level::err, "Streaming thread already running");
        return STREAM_ALREADY_RUN_ERROR;
    }
    if (buffer_size <= 0 || buffer_size > MAX_CAPTURE_SAMPLES)
    {
        safe_logger (spdlog::level::err, "invalid array size");
        return INVALID_BUFFER_SIZE_ERROR;
    }

    if (db)
    {
        delete db;
        db = NULL;
    }

    // start streaming
    if (socket.send ("b", 1) != 1)
    {
        safe_logger (spdlog::level::err, "Failed to send a command to board");
        return BOARD_WRITE_ERROR;
    }

    db = new DataBuffer (num_channels, buffer_size);
    if (!db->is_ready ())
    {
        safe_logger (spdlog::level::err, "unable to prepare buffer");
        return INVALID_BUFFER_SIZE_ERROR;
    }

    keep_alive = true;
    streaming_thread = std::thread ([this] { this->read_thread (); });
    // wait for data to ensure that everything is okay
    std::unique_lock<std::mutex> lk (this->m);
    auto sec = std::chrono::seconds (1);
    if (cv.wait_for (lk, 5 * sec, [this] { return this->state != SYNC_TIMEOUT_ERROR; }))
    {
        this->is_streaming = true;
        return this->state;
    }
    else
    {
        Board::board_logger->error ("no data received in 5sec, stopping thread");
        this->is_streaming = true;
        this->stop_stream ();
        // more likely error occured due to wrong ip address, return UNABLE_TO_OPEN_PORT instead
        // SYNC_TIMEOUT_ERROR
        return UNABLE_TO_OPEN_PORT_ERROR;
    }
}

int NovaXR::stop_stream ()
{
    if (is_streaming)
    {
        keep_alive = false;
        is_streaming = false;
        streaming_thread.join ();
        this->state = SYNC_TIMEOUT_ERROR;
        if (socket.send ("s", 1) != 1)
        {
            safe_logger (spdlog::level::err, "Failed to send a command to board");
            return BOARD_WRITE_ERROR;
        }
        return STATUS_OK;
    }
    else
    {
        return STREAM_THREAD_IS_NOT_RUNNING;
    }
}

int NovaXR::release_session ()
{
    if (initialized)
    {
        if (is_streaming)
        {
            stop_stream ();
        }
        initialized = false;
        socket.close ();
    }
    return STATUS_OK;
}

void NovaXR::read_thread ()
{
    /* ------ NovaXR packet format --------
     * Packet Byte [0]:     Packet Number
     * Packet Byte [1]:     PPG
     * Packet Byte [2:3]:   EDA
     * Packet Byte [4:6]:   EEG_FC 0
     * Packet Byte [7:9]:   EEG_FC 1
     * Packet Byte [10:12]: EEG_OL 0
     * Packet Byte [13:15]: EEG_OL 1
     * Packet Byte [16:18]: EEG_OL 2
     * Packet Byte [19:21]: EEG_OL 3
     * Packet Byte [22:24]: EEG_OL 4
     * Packet Byte [25:27]: EEG_OL 5
     * Packet Byte [28:30]: EEG_OL 6
     * Packet Byte [31:33]: EEG_OL 7
     * Packet Byte [34:36]: EOG 0
     * Packet Byte [37:39]: EOG 1
     * Packet Byte [40:42]: EMG 0
     * Packet Byte [43:45]: EMG 1
     * Packet Byte [46:48]: EMG 2
     * Packet Byte [49:51]: EMG 3
     * Packet Byte [52:53]: AXL X
     * Packet Byte [54:55]: AXL Y
     * Packet Byte [56:57]: AXL Z
     * Packet Byte [58:59]: GYR X
     * Packet Byte [60:61]: GYR Y
     * Packet Byte [62:63]: GYR Z
     * Packet Byte [64:71]: Timestamp
     */

    int res;
    unsigned char b[72];
    while (keep_alive)
    {
        res = socket.recv (b, 72);
        if (res != 72)
        {
            safe_logger (spdlog::level::trace, "unable to read 72 bytes, read {}", res);
            continue;
        }
        else
        {
            // inform main thread that everything is ok and first package was received
            if (this->state != STATUS_OK)
            {
                for (int i = 0; i < 72; i++)
                {
                    safe_logger (spdlog::level::trace, "byte {} val {}", i, b[i]);
                }
                {
                    std::lock_guard<std::mutex> lk (this->m);
                    this->state = STATUS_OK;
                }
                this->cv.notify_one ();
                safe_logger (spdlog::level::debug, "start streaming");
            }
        }

        float package[23];
        // package num
        package[0] = (float)b[0];
        // eeg and emg
        for (int i = 4; i < 18; i++)
        {
            // put them directly after package num in brainflow
            package[i - 3] = eeg_scale * (float)cast_24bit_to_int32 (b + 1 + 3 * i);
        }
        package[15] = (float)b[1];                                // ppg
        package[16] = cast_16bit_to_int32 (b + 2);                // eda todo scale?
        package[17] = accel_scale * cast_16bit_to_int32 (b + 52); // accel x
        package[18] = accel_scale * cast_16bit_to_int32 (b + 54); // accel y
        package[19] = accel_scale * cast_16bit_to_int32 (b + 56); // accel z
        package[20] = gyro_scale * cast_16bit_to_int32 (b + 58);  // gyro x
        package[21] = gyro_scale * cast_16bit_to_int32 (b + 60);  // gyro y
        package[22] = gyro_scale * cast_16bit_to_int32 (b + 62);  // gyro z

        double timestamp = (double)atol ((const char *)(b + 64));
        db->add_data (timestamp, package);
    }
}
