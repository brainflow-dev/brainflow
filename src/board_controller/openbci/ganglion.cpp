#include <chrono>
#include <string.h>
#include <string>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "custom_cast.h"
#include "ganglion.h"
#include "ganglion_interface.h"
#include "openbci_helpers.h"

// sleep is 10 ms, so wait for 2.5sec total
#define MAX_ATTEMPTS_TO_GET_DATA 250

int Ganglion::num_objects = 0;

Ganglion::Ganglion (struct BrainFlowInputParams params) : Board ((int)GANGLION_BOARD, params)
{
    Ganglion::num_objects++;
    if (Ganglion::num_objects > 1)
    {
        this->is_valid = false;
    }
    else
    {
        this->is_valid = true;
    }
    this->use_mac_addr = (params.mac_address.empty ()) ? false : true;
    this->is_streaming = false;
    this->keep_alive = false;
    this->initialized = false;
    this->num_channels = 8;
    this->state = SYNC_TIMEOUT_ERROR;
}

Ganglion::~Ganglion ()
{
    skip_logs = true;
    Ganglion::num_objects--;
    release_session ();
}

int Ganglion::prepare_session ()
{
    if (initialized)
    {
        safe_logger (spdlog::level::info, "Session is already prepared");
        return STATUS_OK;
    }

    if (!this->is_valid)
    {
        safe_logger (spdlog::level::info, "only one ganglion per process is supported");
        return ANOTHER_BOARD_IS_CREATED_ERROR;
    }

    if (this->params.serial_port.empty ())
    {
        safe_logger (spdlog::level::err, "you need to specify dongle port");
        return INVALID_ARGUMENTS_ERROR;
    }

    int res = this->call_init ();
    if (res != STATUS_OK)
    {
        return res;
    }
    safe_logger (spdlog::level::debug, "ganglionlib initialized");

    res = this->call_open ();
    if (res != STATUS_OK)
    {
        return res;
    }

    this->initialized = true;
    return STATUS_OK;
}

int Ganglion::start_stream (int buffer_size, char *streamer_params)
{
    if (this->is_streaming)
    {
        safe_logger (spdlog::level::err, "Streaming thread already running");
        return STREAM_ALREADY_RUN_ERROR;
    }
    if (buffer_size <= 0 || buffer_size > MAX_CAPTURE_SAMPLES)
    {
        safe_logger (spdlog::level::err, "invalid array size");
        return INVALID_BUFFER_SIZE_ERROR;
    }

    if (this->db)
    {
        delete this->db;
        this->db = NULL;
    }
    if (this->streamer)
    {
        delete this->streamer;
        this->streamer = NULL;
    }

    int res = prepare_streamer (streamer_params);
    if (res != STATUS_OK)
    {
        return res;
    }
    this->db = new DataBuffer (num_channels, buffer_size);
    if (!this->db->is_ready ())
    {
        Board::board_logger->error ("unable to prepare buffer with size {}", buffer_size);
        delete db;
        db = NULL;
        return INVALID_BUFFER_SIZE_ERROR;
    }

    return this->start_streaming_prepared ();
}

int Ganglion::start_streaming_prepared ()
{
    int res = this->call_start ();
    if (res != STATUS_OK)
    {
        return res;
    }

    this->keep_alive = true;
    this->streaming_thread = std::thread ([this] { this->read_thread (); });

    // wait for data to ensure that everything is okay
    std::unique_lock<std::mutex> lk (this->m);
    auto sec = std::chrono::seconds (1);
    if (cv.wait_for (lk, 20 * sec, [this] { return this->state != SYNC_TIMEOUT_ERROR; }))
    {
        this->is_streaming = true;
        return this->state;
    }
    else
    {
        safe_logger (spdlog::level::err, "no data received in 20sec, stopping thread");
        this->is_streaming = true;
        this->stop_stream ();
        return SYNC_TIMEOUT_ERROR;
    }
}

int Ganglion::stop_stream ()
{
    if (this->is_streaming)
    {
        this->keep_alive = false;
        this->is_streaming = false;
        this->streaming_thread.join ();
        if (this->streamer)
        {
            delete this->streamer;
            this->streamer = NULL;
        }
        this->state = SYNC_TIMEOUT_ERROR;
        return this->call_stop ();
    }
    else
    {
        return STREAM_THREAD_IS_NOT_RUNNING;
    }
}

int Ganglion::release_session ()
{
    if (this->initialized)
    {
        this->stop_stream ();

        if (this->db)
        {
            delete this->db;
            this->db = NULL;
        }
        this->initialized = false;
    }
    this->call_close ();
    this->call_release ();
    return STATUS_OK;
}

void Ganglion::read_thread ()
{
    // https://docs.openbci.com/Hardware/08-Ganglion_Data_Format
    int num_attempts = 0;
    bool was_reset = false;
    float last_data[8] = {0};

    double accel_x = 0.;
    double accel_y = 0.;
    double accel_z = 0.;

    while (this->keep_alive)
    {
        struct GanglionLib::GanglionData data;
        int res = GanglionLib::get_data ((void *)&data);
        if (res == (int)GanglionLib::CustomExitCodes::STATUS_OK)
        {
            if (this->state != STATUS_OK)
            {
                {
                    std::lock_guard<std::mutex> lk (this->m);
                    this->state = STATUS_OK;
                }
                this->cv.notify_one ();
                safe_logger (spdlog::level::debug, "start streaming");
            }

            double package[8] = {0.f};
            // delta holds 8 nums because (4 by each package)
            float delta[8] = {0.f};
            int bits_per_num = 0;
            unsigned char package_bits[160] = {0}; // 20 * 8
            for (int i = 0; i < 20; i++)
            {
                uchar_to_bits (data.data[i], package_bits + i * 8);
            }

            // no compression, used to init variable
            if (data.data[0] == 0)
            {
                // shift the last data packet to make room for a newer one
                last_data[0] = last_data[4];
                last_data[1] = last_data[5];
                last_data[2] = last_data[6];
                last_data[3] = last_data[7];

                // add new packet
                last_data[4] = cast_24bit_to_int32 (data.data + 1);
                last_data[5] = cast_24bit_to_int32 (data.data + 4);
                last_data[6] = cast_24bit_to_int32 (data.data + 7);
                last_data[7] = cast_24bit_to_int32 (data.data + 10);

                // scale new packet and insert into result
                package[0] = 0.;
                package[1] = this->eeg_scale * last_data[4];
                package[2] = this->eeg_scale * last_data[5];
                package[3] = this->eeg_scale * last_data[6];
                package[4] = this->eeg_scale * last_data[7];
                package[5] = accel_x;
                package[6] = accel_y;
                package[7] = accel_z;
                this->streamer->stream_data (package, 8, data.timestamp);
                this->db->add_data (data.timestamp, package);
                continue;
            }
            // 18 bit compression, sends delta from previous value instead of real value!
            else if ((data.data[0] >= 1) && (data.data[0] <= 100))
            {
                int last_digit = data.data[0] % 10;
                switch (last_digit)
                {
                    case 0:
                        accel_x = this->accel_scale * data.data[19];
                        break;
                    case 1:
                        accel_y = this->accel_scale * data.data[19];
                        break;
                    case 2:
                        accel_z = this->accel_scale * data.data[19];
                        break;
                    default:
                        break;
                }
                bits_per_num = 18;
            }
            else if ((data.data[0] >= 101) && (data.data[0] <= 200))
            {
                bits_per_num = 19;
            }
            else if (data.data[0] > 200)
            {
                safe_logger (
                    spdlog::level::warn, "unexpected value {} in first byte", data.data[0]);
                continue;
            }
            // handle compressed data for 18 or 19 bits
            for (int i = 8, counter = 0; i < bits_per_num * 8; i += bits_per_num, counter++)
            {
                if (bits_per_num == 18)
                {
                    delta[counter] = cast_ganglion_bits_to_int32<18> (package_bits + i);
                }
                else
                {
                    delta[counter] = cast_ganglion_bits_to_int32<19> (package_bits + i);
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
            package[0] = data.data[0];
            package[1] = this->eeg_scale * last_data[0];
            package[2] = this->eeg_scale * last_data[1];
            package[3] = this->eeg_scale * last_data[2];
            package[4] = this->eeg_scale * last_data[3];
            package[5] = accel_x;
            package[6] = accel_y;
            package[7] = accel_z;
            this->streamer->stream_data (package, 8, data.timestamp);
            this->db->add_data (data.timestamp, package);
            // add second package
            package[1] = this->eeg_scale * last_data[4];
            package[2] = this->eeg_scale * last_data[5];
            package[3] = this->eeg_scale * last_data[6];
            package[4] = this->eeg_scale * last_data[7];
            this->streamer->stream_data (package, 8, data.timestamp);
            this->db->add_data (data.timestamp, package);
        }
        else
        {
            if (this->state == SYNC_TIMEOUT_ERROR)
            {
                num_attempts++;
            }
            if (num_attempts == MAX_ATTEMPTS_TO_GET_DATA)
            {
                safe_logger (spdlog::level::err, "no data received");
                {
                    std::lock_guard<std::mutex> lk (this->m);
                    this->state = GENERAL_ERROR;
                }
                this->cv.notify_one ();
                return;
            }
#ifdef _WIN32
            Sleep (10);
#else
            usleep (10000);
#endif
        }
    }
}

int Ganglion::config_board (char *config)
{
    safe_logger (spdlog::level::debug, "Trying to config Ganglion with {}", config);
    int res = validate_config (config);
    if (res != STATUS_OK)
    {
        return res;
    }
    // need to pause, config and restart. I have no idea why it doesnt work if I restart it inside
    // bglib or just call call_stop call_start, full restart solves the issue
    if (this->keep_alive)
    {
        safe_logger (spdlog::level::info, "stoping streaming to configure board");
        // stop stream
        this->keep_alive = false;
        this->is_streaming = false;
        this->streaming_thread.join ();
        this->state = SYNC_TIMEOUT_ERROR;
        int res = this->call_stop ();
        if (res != STATUS_OK)
        {
            return res;
        }
        // call itself with disabled streaming
        res = config_board (config);
        if (res != STATUS_OK)
        {
            return res;
        }
        return this->start_streaming_prepared ();
    }
    else
    {
        return this->call_config (config);
    }
}

int Ganglion::call_init ()
{
    int res = GanglionLib::initialize ((void *)params.serial_port.c_str ());
    if (res != (int)GanglionLib::CustomExitCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to init GanglionLib {}", res);
        return GENERAL_ERROR;
    }
    return STATUS_OK;
}

int Ganglion::call_open ()
{
    int res = GanglionLib::CustomExitCodes::GENERAL_ERROR;
    if (this->use_mac_addr)
    {
        safe_logger (spdlog::level::info, "search for {}", this->params.mac_address.c_str ());
        res =
            GanglionLib::open_ganglion_mac_addr (const_cast<char *> (params.mac_address.c_str ()));
    }
    else
    {
        safe_logger (
            spdlog::level::info, "mac address is not specified, try to find ganglion without it");
        res = GanglionLib::open_ganglion (NULL);
    }
    if (res != GanglionLib::CustomExitCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to Open Ganglion Device {}", res);
        return GENERAL_ERROR;
    }
    return STATUS_OK;
}

int Ganglion::call_config (char *config)
{
    int res = GanglionLib::config_board (config);
    if (res != GanglionLib::CustomExitCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to config board {}", res);
        return GENERAL_ERROR;
    }
    return STATUS_OK;
}

int Ganglion::call_start ()
{
    int res = GanglionLib::start_stream (NULL);
    if (res != (int)GanglionLib::CustomExitCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to start streaming {}", res);
        return GENERAL_ERROR;
    }
    return STATUS_OK;
}

int Ganglion::call_stop ()
{
    int res = GanglionLib::stop_stream (NULL);
    if (res != (int)GanglionLib::CustomExitCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to stop streaming {}", res);
        return GENERAL_ERROR;
    }
    return STATUS_OK;
}

int Ganglion::call_close ()
{
    int res = GanglionLib::close_ganglion (NULL);
    if (res != (int)GanglionLib::CustomExitCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to close ganglion {}", res);
        return GENERAL_ERROR;
    }
    return STATUS_OK;
}

int Ganglion::call_release ()
{
    int res = GanglionLib::release (NULL);
    if (res != (int)GanglionLib::CustomExitCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to release ganglion library {}", res);
        return GENERAL_ERROR;
    }
    return STATUS_OK;
}
