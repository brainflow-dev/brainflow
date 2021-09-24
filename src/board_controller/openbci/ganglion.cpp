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
#include "ganglion_types.h"
#include "get_dll_dir.h"


int Ganglion::num_objects = 0;


Ganglion::Ganglion (struct BrainFlowInputParams params)
    : Board ((int)BoardIds::GANGLION_BOARD, params)
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
    state = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
    start_command = "b";
    stop_command = "s";

    std::string ganglionlib_path = "";
    std::string ganglionlib_name = "";
    char ganglionlib_dir[1024];
    bool res = get_dll_path (ganglionlib_dir);
#ifdef _WIN32
    if (sizeof (void *) == 4)
    {
        ganglionlib_name = "GanglionLib32.dll";
    }
    else
    {
        ganglionlib_name = "GanglionLib.dll";
    }
#endif
#ifdef __linux__
    ganglionlib_name = "libGanglionLib.so";
#endif
#ifdef __APPLE__
    ganglionlib_name = "libGanglionLib.dylib";
#endif

    if (res)
    {
        ganglionlib_path = std::string (ganglionlib_dir) + ganglionlib_name;
    }
    else
    {
        ganglionlib_path = ganglionlib_name;
    }

    safe_logger (spdlog::level::debug, "use dyn lib: {}", ganglionlib_path.c_str ());
    dll_loader = new DLLLoader (ganglionlib_path.c_str ());
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
        return (int)BrainFlowExitCodes::STATUS_OK;
    }

    if (!is_valid)
    {
        safe_logger (spdlog::level::info, "only one ganglion per process is supported");
        return (int)BrainFlowExitCodes::ANOTHER_BOARD_IS_CREATED_ERROR;
    }

    if (!dll_loader->load_library ())
    {
        safe_logger (spdlog::level::err, "Failed to load library");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    safe_logger (spdlog::level::debug, "Library is loaded");

    if ((params.timeout < 0) || (params.timeout > 600))
    {
        safe_logger (spdlog::level::err, "wrong value for timeout");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
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
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    int res = call_init ();
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }
    safe_logger (spdlog::level::debug, "ganglionlib initialized");

    res = call_open ();
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }

    initialized = true;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int Ganglion::start_stream (int buffer_size, const char *streamer_params)
{
    if (is_streaming)
    {
        safe_logger (spdlog::level::err, "Streaming thread already running");
        return (int)BrainFlowExitCodes::STREAM_ALREADY_RUN_ERROR;
    }

    int res = prepare_for_acquisition (buffer_size, streamer_params);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }

    return start_streaming_prepared ();
}

int Ganglion::start_streaming_prepared ()
{
    int res = call_start ();
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }

    keep_alive = true;
    streaming_thread = std::thread ([this] { read_thread (); });

    // wait for data to ensure that everything is okay
    std::unique_lock<std::mutex> lk (m);
    auto sec = std::chrono::seconds (1);
    if (cv.wait_for (lk, params.timeout * sec,
            [this] { return state != (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR; }))
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
        return (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
    }
}

int Ganglion::stop_stream ()
{
    if (is_streaming)
    {
        keep_alive = false;
        is_streaming = false;
        streaming_thread.join ();
        state = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
        return call_stop ();
    }
    else
    {
        return (int)BrainFlowExitCodes::STREAM_THREAD_IS_NOT_RUNNING;
    }
}

int Ganglion::release_session ()
{
    if (initialized)
    {
        stop_stream ();
        initialized = false;
    }
    free_packages ();
    call_close ();
    call_release ();

    if (dll_loader != NULL)
    {
        dll_loader->free_library ();
        delete dll_loader;
        dll_loader = NULL;
    }

    return (int)BrainFlowExitCodes::STATUS_OK;
}

void Ganglion::read_thread ()
{
    // https://docs.openbci.com/Hardware/08-Ganglion_Data_Format
    int num_attempts = 0;
    int sleep_time = 10;
    int max_attempts = params.timeout * 1000 / sleep_time;
    float last_data[8] = {0};

    double accel_x = 0.;
    double accel_y = 0.;
    double accel_z = 0.;

    double resist_ref = 0.0;
    double resist_first = 0.0;
    double resist_second = 0.0;
    double resist_third = 0.0;
    double resist_fourth = 0.0;

    int (*func) (void *) = (int (*) (void *))dll_loader->get_address ("get_data");
    if (func == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for get_data");
        return;
    }

    int num_rows = board_descr["num_rows"];
    double *package = new double[num_rows];

    while (keep_alive)
    {
        for (int i = 0; i < num_rows; i++)
        {
            package[i] = 0.0;
        }

        struct GanglionLib::GanglionData data;
        int res = func ((void *)&data);
        if (res == (int)GanglionLib::CustomExitCodes::STATUS_OK)
        {
            if (state != (int)BrainFlowExitCodes::STATUS_OK)
            {
                {
                    std::lock_guard<std::mutex> lk (m);
                    state = (int)BrainFlowExitCodes::STATUS_OK;
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
                last_data[4] = (float)cast_24bit_to_int32 (data.data + 1);
                last_data[5] = (float)cast_24bit_to_int32 (data.data + 4);
                last_data[6] = (float)cast_24bit_to_int32 (data.data + 7);
                last_data[7] = (float)cast_24bit_to_int32 (data.data + 10);

                // scale new packet and insert into result
                package[board_descr["package_num_channel"].get<int> ()] = 0.;
                package[board_descr["eeg_channels"][0].get<int> ()] = eeg_scale * last_data[4];
                package[board_descr["eeg_channels"][1].get<int> ()] = eeg_scale * last_data[5];
                package[board_descr["eeg_channels"][2].get<int> ()] = eeg_scale * last_data[6];
                package[board_descr["eeg_channels"][3].get<int> ()] = eeg_scale * last_data[7];
                package[board_descr["accel_channels"][0].get<int> ()] = accel_x;
                package[board_descr["accel_channels"][1].get<int> ()] = accel_y;
                package[board_descr["accel_channels"][2].get<int> ()] = accel_z;
                package[board_descr["timestamp_channel"].get<int> ()] = data.timestamp;
                push_package (package);
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
                package[board_descr["package_num_channel"].get<int> ()] = data.data[0];
                package[board_descr["resistance_channels"][0].get<int> ()] = resist_first;
                package[board_descr["resistance_channels"][1].get<int> ()] = resist_second;
                package[board_descr["resistance_channels"][2].get<int> ()] = resist_third;
                package[board_descr["resistance_channels"][3].get<int> ()] = resist_fourth;
                package[board_descr["resistance_channels"][4].get<int> ()] = resist_ref;
                package[board_descr["timestamp_channel"].get<int> ()] = data.timestamp;
                push_package (package);
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
            package[board_descr["package_num_channel"].get<int> ()] = data.data[0];
            package[board_descr["eeg_channels"][0].get<int> ()] = eeg_scale * last_data[0];
            package[board_descr["eeg_channels"][1].get<int> ()] = eeg_scale * last_data[1];
            package[board_descr["eeg_channels"][2].get<int> ()] = eeg_scale * last_data[2];
            package[board_descr["eeg_channels"][3].get<int> ()] = eeg_scale * last_data[3];
            package[board_descr["accel_channels"][0].get<int> ()] = accel_x;
            package[board_descr["accel_channels"][1].get<int> ()] = accel_y;
            package[board_descr["accel_channels"][2].get<int> ()] = accel_z;
            package[board_descr["timestamp_channel"].get<int> ()] = data.timestamp;
            push_package (package);
            // add second package
            package[board_descr["eeg_channels"][0].get<int> ()] = eeg_scale * last_data[4];
            package[board_descr["eeg_channels"][1].get<int> ()] = eeg_scale * last_data[5];
            package[board_descr["eeg_channels"][2].get<int> ()] = eeg_scale * last_data[6];
            package[board_descr["eeg_channels"][3].get<int> ()] = eeg_scale * last_data[7];
            package[board_descr["timestamp_channel"].get<int> ()] = data.timestamp;
            push_package (package);
        }
        else
        {
            if (state == (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR)
            {
                num_attempts++;
            }
            if (num_attempts == max_attempts)
            {
                safe_logger (spdlog::level::err, "no data received");
                {
                    std::lock_guard<std::mutex> lk (m);
                    state = (int)BrainFlowExitCodes::GENERAL_ERROR;
                }
                cv.notify_one ();
                break;
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

int Ganglion::config_board (std::string config, std::string &response)
{
    const char *conf = config.c_str ();
    safe_logger (spdlog::level::debug, "Trying to config Ganglion with {}", conf);
    // need to pause, config and restart. I have no idea why it doesnt work if I restart it inside
    // bglib or just call call_stop call_start, full restart solves the issue
    if (keep_alive)
    {
        safe_logger (spdlog::level::info, "stoping streaming to configure board");
        // stop stream
        keep_alive = false;
        is_streaming = false;
        streaming_thread.join ();
        state = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
        int res = call_stop ();
        if (res != (int)BrainFlowExitCodes::STATUS_OK)
        {
            return res;
        }
        /* hack, in ganglion commands to enable/disable impedance check control data streaming
        to handle it and keep it consistent with other devices we swap chars for command_start and
        command_stop
        */
        if ((strlen (conf)) && (conf[0] == 'z'))
        {
            start_command = "z";
            stop_command = "Z";
        }
        else
        {
            if ((strlen (conf)) && (conf[0] == 'Z'))
            {
                start_command = "b";
                stop_command = "s";
            }
            // plain command which doesnt change streaming behaviour
            else
            {
                // call itself with disabled streaming
                res = config_board (config, response);
            }
        }
        if (res != (int)BrainFlowExitCodes::STATUS_OK)
        {
            return res;
        }
        return start_streaming_prepared ();
    }
    // streaming is not started, dont pause
    else
    {
        if ((strlen (conf)) && (conf[0] == 'z'))
        {
            start_command = "z";
            stop_command = "Z";
        }
        else
        {
            if ((strlen (conf)) && (conf[0] == 'Z'))
            {
                start_command = "b";
                stop_command = "s";
            }
            else
            {
                return call_config (const_cast<char *> (conf));
            }
        }
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int Ganglion::call_init ()
{
    if (dll_loader == NULL)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    int (*func) (void *) = (int (*) (void *))dll_loader->get_address ("initialize");
    if (func == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for initialize");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }

    struct GanglionLib::GanglionInputData input_data (params.timeout, params.serial_port.c_str ());
    int res = func ((void *)&input_data);
    if (res != (int)GanglionLib::CustomExitCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to init GanglionLib {}", res);
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int Ganglion::call_open ()
{
    if (dll_loader == NULL)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    int res = GanglionLib::CustomExitCodes::GENERAL_ERROR;
    if (use_mac_addr)
    {
        int (*func) (void *) = (int (*) (void *))dll_loader->get_address ("open_ganglion_mac_addr");
        if (func == NULL)
        {
            safe_logger (
                spdlog::level::err, "failed to get function address for open_ganglion_mac_addr");
            return (int)BrainFlowExitCodes::GENERAL_ERROR;
        }

        safe_logger (spdlog::level::info, "search for {}", params.mac_address.c_str ());
        res = func (const_cast<char *> (params.mac_address.c_str ()));
    }
    else
    {
        int (*func) (void *) = (int (*) (void *))dll_loader->get_address ("open_ganglion");
        if (func == NULL)
        {
            safe_logger (spdlog::level::err, "failed to get function address for open_ganglion");
            return (int)BrainFlowExitCodes::GENERAL_ERROR;
        }

        safe_logger (
            spdlog::level::info, "mac address is not specified, try to find ganglion without it");
        res = func (NULL);
    }
    if (res != GanglionLib::CustomExitCodes::STATUS_OK)
    {
        if (res == GanglionLib::CustomExitCodes::PORT_OPEN_ERROR)
        {
            safe_logger (spdlog::level::err,
                "Make sure you provided correct port name and have permissions to open it(run with "
                "sudo/admin). And close all other apps using this port.");
        }
        safe_logger (spdlog::level::err, "failed to Open Ganglion Device {}", res);
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int Ganglion::call_config (char *config)
{
    if (dll_loader == NULL)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    int (*func) (void *) = (int (*) (void *))dll_loader->get_address ("config_board");
    if (func == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for config_board");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }

    int res = func (config);
    if (res != GanglionLib::CustomExitCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to config board {}", res);
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int Ganglion::call_start ()
{
    if (dll_loader == NULL)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    int (*func) (void *) = (int (*) (void *))dll_loader->get_address ("start_stream");
    if (func == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for start_stream");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }

    safe_logger (spdlog::level::info, "use command {} to start streaming", start_command.c_str ());
    int res = func ((void *)start_command.c_str ());
    if (res != (int)GanglionLib::CustomExitCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to start streaming {}", res);
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int Ganglion::call_stop ()
{
    if (dll_loader == NULL)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    int (*func) (void *) = (int (*) (void *))dll_loader->get_address ("stop_stream");
    if (func == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for stop_stream");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }

    int res = func ((void *)stop_command.c_str ());
    if (res != (int)GanglionLib::CustomExitCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to stop streaming {}", res);
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int Ganglion::call_close ()
{
    if (dll_loader == NULL)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    int (*func) (void *) = (int (*) (void *))dll_loader->get_address ("close_ganglion");
    if (func == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for close_ganglion");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }

    int res = func (NULL);
    if (res != (int)GanglionLib::CustomExitCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to close ganglion {}", res);
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int Ganglion::call_release ()
{
    if (dll_loader == NULL)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    int (*func) (void *) = (int (*) (void *))dll_loader->get_address ("release");
    if (func == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for release");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }

    int res = func (NULL);
    if (res != (int)GanglionLib::CustomExitCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to release ganglion library {}", res);
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}
