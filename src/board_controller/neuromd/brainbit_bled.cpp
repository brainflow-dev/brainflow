#include <chrono>
#include <string.h>
#include <string>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "brainbit_bled.h"
#include "brainbit_types.h"
#include "custom_cast.h"
#include "get_dll_dir.h"

int BrainBitBLED::num_objects = 0;


BrainBitBLED::BrainBitBLED (struct BrainFlowInputParams params)
    : Board ((int)BoardIds::BRAINBIT_BLED_BOARD, params)
{
    BrainBitBLED::num_objects++;

    if (BrainBitBLED::num_objects > 1)
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

    std::string brainbitlib_path = "";
    std::string brainbitlib_name = "";
    char brainbitlib_dir[1024];
    bool res = get_dll_path (brainbitlib_dir);

#ifdef _WIN32
    if (sizeof (void *) == 4)
    {
        brainbitlib_name = "BrainBitLib32.dll";
    }
    else
    {
        brainbitlib_name = "BrainBitLib.dll";
    }
#endif
#ifdef __linux__
    brainbitlib_name = "libBrainBitLib.so";
#endif
#ifdef __APPLE__
    brainbitlib_name = "libBrainBitLib.dylib";
#endif

    if (res)
    {
        brainbitlib_path = std::string (brainbitlib_dir) + brainbitlib_name;
    }
    else
    {
        brainbitlib_path = brainbitlib_name;
    }

    safe_logger (spdlog::level::debug, "use dyn lib: {}", brainbitlib_path.c_str ());
    dll_loader = new DLLLoader (brainbitlib_path.c_str ());
}

BrainBitBLED::~BrainBitBLED ()
{
    skip_logs = true;
    BrainBitBLED::num_objects--;
    release_session ();
}

int BrainBitBLED::prepare_session ()
{
    if (initialized)
    {
        safe_logger (spdlog::level::info, "Session is already prepared");
        return (int)BrainFlowExitCodes::STATUS_OK;
    }

    if (!is_valid)
    {
        safe_logger (spdlog::level::info, "only one BrainBitBLED per process is allowed");
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
    safe_logger (spdlog::level::debug, "brainbitlib initialized");
    res = call_open ();
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }
    initialized = true;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int BrainBitBLED::start_stream (int buffer_size, char *streamer_params)
{
    if (is_streaming)
    {
        safe_logger (spdlog::level::err, "Streaming thread already running");
        return (int)BrainFlowExitCodes::STREAM_ALREADY_RUN_ERROR;
    }
    if (buffer_size <= 0 || buffer_size > MAX_CAPTURE_SAMPLES)
    {
        safe_logger (spdlog::level::err, "invalid array size");
        return (int)BrainFlowExitCodes::INVALID_BUFFER_SIZE_ERROR;
    }

    int res = prepare_for_acquisition (buffer_size, streamer_params);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }

    res = call_start ();
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

int BrainBitBLED::stop_stream ()
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

int BrainBitBLED::release_session ()
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

void BrainBitBLED::read_thread ()
{
    int num_attempts = 0;
    int sleep_time = 10;
    int max_attempts = params.timeout * 1000 / sleep_time;

    int (*func) (void *) = (int (*) (void *))dll_loader->get_address ("get_data");
    if (func == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for get_data");
        return;
    }

    while (keep_alive)
    {
        struct BrainBitBLEDLib::BrainBitData data;
        int res = func ((void *)&data);
        if (res == (int)BrainBitBLEDLib::CustomExitCodes::STATUS_OK)
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
            push_package (data.data);
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
}

int BrainBitBLED::config_board (std::string config, std::string &response)
{
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

int BrainBitBLED::call_init ()
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

    struct BrainBitBLEDLib::BrainBitInputData input_data (
        params.timeout, params.serial_port.c_str ());
    int res = func ((void *)&input_data);
    if (res != (int)BrainBitBLEDLib::CustomExitCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to initialize {}", res);
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int BrainBitBLED::call_open ()
{
    if (dll_loader == NULL)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    int res = BrainBitBLEDLib::CustomExitCodes::GENERAL_ERROR;
    if (use_mac_addr)
    {
        int (*func) (void *) = (int (*) (void *))dll_loader->get_address ("open_device_mac_addr");
        if (func == NULL)
        {
            safe_logger (
                spdlog::level::err, "failed to get function address for open_device_mac_addr");
            return (int)BrainFlowExitCodes::GENERAL_ERROR;
        }

        safe_logger (spdlog::level::info, "search for {}", params.mac_address.c_str ());
        res = func (const_cast<char *> (params.mac_address.c_str ()));
    }
    else
    {
        int (*func) (void *) = (int (*) (void *))dll_loader->get_address ("open_device");
        if (func == NULL)
        {
            safe_logger (spdlog::level::err, "failed to get function address for open_device");
            return (int)BrainFlowExitCodes::GENERAL_ERROR;
        }

        safe_logger (
            spdlog::level::info, "mac address is not specified, try to find brainbit without it");
        res = func (NULL);
    }
    if (res != BrainBitBLEDLib::CustomExitCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to Open BrainBitBLED Device {}", res);
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int BrainBitBLED::call_start ()
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

    int res = func (NULL);
    if (res != (int)BrainBitBLEDLib::CustomExitCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to start streaming {}", res);
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int BrainBitBLED::call_stop ()
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

    int res = func (NULL);
    if (res != (int)BrainBitBLEDLib::CustomExitCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to stop streaming {}", res);
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int BrainBitBLED::call_close ()
{
    if (dll_loader == NULL)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    int (*func) (void *) = (int (*) (void *))dll_loader->get_address ("close_device");
    if (func == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for close_device");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }

    int res = func (NULL);
    if (res != (int)BrainBitBLEDLib::CustomExitCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to close brainbit {}", res);
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int BrainBitBLED::call_release ()
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
    if (res != (int)BrainBitBLEDLib::CustomExitCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to release brainbit library {}", res);
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}