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


int Ganglion::num_objects = 0;


Ganglion::Ganglion (struct BrainFlowInputParams params) : Board ((int)GANGLION_BOARD, params)
{
    Ganglion::num_objects++;
    if (Ganglion::num_objects > 1)
    {
        is_valid = false;
    }
    else
    {
        is_valid = true;
    }
    use_mac_addr = (params.mac_address.empty ()) ? false : true;
    is_streaming = false;
    keep_alive = false;
    initialized = false;
    num_channels = 13;
    state = SYNC_TIMEOUT_ERROR;
    start_command = "b";
    stop_command = "s";
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

    if (!is_valid)
    {
        safe_logger (spdlog::level::info, "only one ganglion per process is supported");
        return ANOTHER_BOARD_IS_CREATED_ERROR;
    }

    if ((params.timeout < 0) || (params.timeout > 600))
    {
        safe_logger (spdlog::level::err, "wrong value for timeout");
        return INVALID_ARGUMENTS_ERROR;
    }
    if (params.timeout == 0)
    {
        params.timeout = 15;
    }
    safe_logger (spdlog::level::info, "use {} as a timeout for device discovery and for callbacks",
        params.timeout);

    if (params.serial_port.empty ())
    {
        safe_logger (spdlog::level::err, "you need to specify dongle port");
        return INVALID_ARGUMENTS_ERROR;
    }

    int res = call_init ();
    if (res != STATUS_OK)
    {
        return res;
    }
    safe_logger (spdlog::level::debug, "ganglionlib initialized");

    res = call_open ();
    if (res != STATUS_OK)
    {
        return res;
    }

    initialized = true;
    return STATUS_OK;
}

int Ganglion::start_stream (int buffer_size, char *streamer_params)
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
    if (streamer)
    {
        delete streamer;
        streamer = NULL;
    }

    int res = prepare_streamer (streamer_params);
    if (res != STATUS_OK)
    {
        return res;
    }
    db = new DataBuffer (num_channels, buffer_size);
    if (!db->is_ready ())
    {
        Board::board_logger->error ("unable to prepare buffer with size {}", buffer_size);
        delete db;
        db = NULL;
        return INVALID_BUFFER_SIZE_ERROR;
    }

    return start_streaming_prepared ();
}

int Ganglion::start_streaming_prepared ()
{
    int res = call_start ();
    if (res != STATUS_OK)
    {
        return res;
    }

    keep_alive = true;
    streaming_thread = std::thread ([this] { read_thread (); });

    // wait for data to ensure that everything is okay
    std::unique_lock<std::mutex> lk (m);
    auto sec = std::chrono::seconds (1);
    if (cv.wait_for (lk, params.timeout * sec, [this] { return state != SYNC_TIMEOUT_ERROR; }))
    {
        is_streaming = true;
        return state;
    }
    else
    {
        safe_logger (
            spdlog::level::err, "no data received in {} sec, stopping thread", params.timeout);
        is_streaming = true;
        stop_stream ();
        return SYNC_TIMEOUT_ERROR;
    }
}

int Ganglion::stop_stream ()
{
    if (is_streaming)
    {
        keep_alive = false;
        is_streaming = false;
        streaming_thread.join ();
        if (streamer)
        {
            delete streamer;
            streamer = NULL;
        }
        state = SYNC_TIMEOUT_ERROR;
        return call_stop ();
    }
    else
    {
        return STREAM_THREAD_IS_NOT_RUNNING;
    }
}

int Ganglion::release_session ()
{
    if (initialized)
    {
        stop_stream ();
        initialized = false;
    }
    call_close ();
    call_release ();
    return STATUS_OK;
}

void Ganglion::read_thread ()
{
    // https://docs.openbci.com/Hardware/08-Ganglion_Data_Format
    int num_attempts = 0;
    int sleep_time = 10;
    int max_attempts = params.timeout * 1000 / sleep_time;
    bool was_reset = false;
    float last_data[8] = {0};

    double accel_x = 0.;
    double accel_y = 0.;
    double accel_z = 0.;

    double resist_ref = 0.0;
    double resist_first = 0.0;
    double resist_second = 0.0;
    double resist_third = 0.0;
    double resist_fourth = 0.0;

    double *package = new double[num_channels];

    while (keep_alive)
    {
        for (int i = 0; i < num_channels; i++)
        {
            package[i] = 0.0;
        }

        struct GanglionLib::GanglionData data;
        int res = GanglionLib::get_data ((void *)&data);
        if (res == (int)GanglionLib::CustomExitCodes::STATUS_OK)
        {
            if (state != STATUS_OK)
            {
                {
                    std::lock_guard<std::mutex> lk (m);
                    state = STATUS_OK;
                }
                cv.notify_one ();
                safe_logger (spdlog::level::debug, "start streaming");
            }

            // delta holds 8 nums (4 by each package)
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
                package[1] = eeg_scale * last_data[4];
                package[2] = eeg_scale * last_data[5];
                package[3] = eeg_scale * last_data[6];
                package[4] = eeg_scale * last_data[7];
                package[5] = accel_x;
                package[6] = accel_y;
                package[7] = accel_z;
                streamer->stream_data (package, num_channels, data.timestamp);
                db->add_data (data.timestamp, package);
                continue;
            }
            // 18 bit compression, sends delta from previous value instead of real value!
            else if ((data.data[0] >= 1) && (data.data[0] <= 100))
            {
                int last_digit = data.data[0] % 10;
                switch (last_digit)
                {
                    // accel data is signed, so we must cast it to signed char
                    // due to a known bug in ganglion firmware, we must swap x and z, and invert z.
                    case 0:
                        accel_z = -accel_scale * (char)data.data[19];
                        break;
                    case 1:
                        accel_y = accel_scale * (char)data.data[19];
                        break;
                    case 2:
                        accel_x = accel_scale * (char)data.data[19];
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
            else if ((data.data[0] > 200) && (data.data[0] < 206))
            {
                // asci sting with value and 'Z' in the end
                int val = 0;
                int i = 0;
                for (i = 1; i < 6; i++)
                {
                    if (data.data[i] == 'Z')
                    {
                        break;
                    }
                }
                std::string asci_value ((const char *)(data.data + 1), i - 1);

                try
                {
                    val = std::stoi (asci_value);
                }
                catch (...)
                {
                    safe_logger (spdlog::level::err, "failed to parse impedance data: {}",
                        asci_value.c_str ());
                    continue;
                }

                switch (data.data[0] % 10)
                {
                    case 1:
                        resist_first = val;
                        break;
                    case 2:
                        resist_second = val;
                        break;
                    case 3:
                        resist_third = val;
                        break;
                    case 4:
                        resist_fourth = val;
                        break;
                    case 5:
                        resist_ref = val;
                        break;
                    default:
                        break;
                }
                package[0] = data.data[0];
                package[8] = resist_first;
                package[9] = resist_second;
                package[10] = resist_third;
                package[11] = resist_fourth;
                package[12] = resist_ref;
                streamer->stream_data (package, num_channels, data.timestamp);
                db->add_data (data.timestamp, package);
                continue;
            }
            else
            {
                for (int i = 0; i < 20; i++)
                {
                    safe_logger (spdlog::level::warn, "byte {} value {}", i, data.data[i]);
                }
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
            package[1] = eeg_scale * last_data[0];
            package[2] = eeg_scale * last_data[1];
            package[3] = eeg_scale * last_data[2];
            package[4] = eeg_scale * last_data[3];
            package[5] = accel_x;
            package[6] = accel_y;
            package[7] = accel_z;
            streamer->stream_data (package, num_channels, data.timestamp);
            db->add_data (data.timestamp, package);
            // add second package
            package[1] = eeg_scale * last_data[4];
            package[2] = eeg_scale * last_data[5];
            package[3] = eeg_scale * last_data[6];
            package[4] = eeg_scale * last_data[7];
            streamer->stream_data (package, num_channels, data.timestamp);
            db->add_data (data.timestamp, package);
        }
        else
        {
            if (state == SYNC_TIMEOUT_ERROR)
            {
                num_attempts++;
            }
            if (num_attempts == max_attempts)
            {
                safe_logger (spdlog::level::err, "no data received");
                {
                    std::lock_guard<std::mutex> lk (m);
                    state = GENERAL_ERROR;
                }
                cv.notify_one ();
                return;
            }
#ifdef _WIN32
            Sleep (sleep_time);
#else
            usleep (sleep_time * 1000);
#endif
        }
    }
    delete[] package;
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
    if (keep_alive)
    {
        safe_logger (spdlog::level::info, "stoping streaming to configure board");
        // stop stream
        keep_alive = false;
        is_streaming = false;
        streaming_thread.join ();
        state = SYNC_TIMEOUT_ERROR;
        int res = call_stop ();
        if (res != STATUS_OK)
        {
            return res;
        }
        /* hack, in ganglion commands to enable/disable impedance check control data streaming
        to handle it and keep it consistent with other devices we swap chars for command_start and
        command_stop
        */
        if ((strlen (config)) && (config[0] == 'z'))
        {
            start_command = "z";
            stop_command = "Z";
        }
        else
        {
            if ((strlen (config)) && (config[0] == 'Z'))
            {
                start_command = "b";
                stop_command = "s";
            }
            // plain command which doesnt change streaming behaviour
            else
            {
                // call itself with disabled streaming
                res = config_board (config);
            }
        }
        if (res != STATUS_OK)
        {
            return res;
        }
        return start_streaming_prepared ();
    }
    // streaming is not started, dont pause
    else
    {
        if ((strlen (config)) && (config[0] == 'z'))
        {
            start_command = "z";
            stop_command = "Z";
        }
        else
        {
            if ((strlen (config)) && (config[0] == 'Z'))
            {
                start_command = "b";
                stop_command = "s";
            }
            else
            {
                return call_config (config);
            }
        }
    }
    return STATUS_OK;
}

int Ganglion::call_init ()
{
    struct GanglionLib::GanglionInputData input_data (params.timeout, params.serial_port.c_str ());
    int res = GanglionLib::initialize ((void *)&input_data);
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
    if (use_mac_addr)
    {
        safe_logger (spdlog::level::info, "search for {}", params.mac_address.c_str ());
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
    safe_logger (spdlog::level::info, "use command {} to start streaming", start_command.c_str ());
    int res = GanglionLib::start_stream ((void *)start_command.c_str ());
    if (res != (int)GanglionLib::CustomExitCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to start streaming {}", res);
        return GENERAL_ERROR;
    }
    return STATUS_OK;
}

int Ganglion::call_stop ()
{
    int res = GanglionLib::stop_stream ((void *)stop_command.c_str ());
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
