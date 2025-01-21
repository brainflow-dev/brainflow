#include <iostream>
#include <string.h>
#include <tuple>

#include "synchroni_board.h"

#include "custom_cast.h"
#include "get_dll_dir.h"
#include "timestamp.h"

using namespace std;

std::shared_ptr<DLLLoader> SynchroniBoard::g_dll_loader = NULL;
SynchroniBoard::SynchroniBoard (int board_id, struct BrainFlowInputParams params)
    : Board (board_id, params)
{
    is_streaming = false;
    keep_alive = false;
    initialized = false;
    state = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
    dll_loader = NULL;
}

SynchroniBoard::~SynchroniBoard ()
{
    skip_logs = true;
    release_session ();
}

std::string SynchroniBoard::get_lib_name ()
{
    std::string synclib_path = "";
    std::string synclib_name = "";
    char synclib_dir[1024];
    bool res = get_dll_path (synclib_dir);

#ifdef _WIN32
    if (sizeof (void *) == 4)
    {
        synclib_name = "SynchroniLib32.dll";
    }
    else
    {
        synclib_name = "SynchroniLib.dll";
    }
#endif
#ifdef __linux__
#if defined(__arm__) || defined(_M_ARM64) || defined(_M_ARM) || defined(__aarch64__)
    synclib_name = "libSynchroniLib_arm64.so";
#else
    synclib_name = "libSynchroniLib_x64.so";
#endif
#endif
#ifdef __APPLE__
    synclib_name = "libSynchroniLib.dylib";
#endif

    if (res)
    {
        synclib_path = std::string (synclib_dir) + synclib_name;
    }
    else
    {
        synclib_path = synclib_name;
    }
    return synclib_path;
}


void SynchroniBoard::read_thread ()
{
    int num_attempts = 0;
    int sleep_time = 10;
    int max_attempts = params.timeout * 1000 / sleep_time;

    int (*func) (void *) = (int (*) (void *))dll_loader->get_address ("synchroni_get_data_default");
    if (func == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for get_data");
        state = (int)BrainFlowExitCodes::GENERAL_ERROR;
        return;
    }

    int num_rows = board_descr["default"]["num_rows"];
    double *data = new double[num_rows];
    if (data == NULL)
    {
        safe_logger (spdlog::level::err, "failed to allocate data");
        state = (int)BrainFlowExitCodes::GENERAL_ERROR;
        return;
    }
    for (int i = 0; i < num_rows; i++)
    {
        data[i] = 0.0;
    }

    std::tuple<std::string, double *, int> info =
        std::make_tuple (params.mac_address, data, num_rows);

    while (keep_alive)
    {
        int res = func ((void *)&info);

        if (res == (int)BrainFlowExitCodes::STATUS_OK)
        {
            if (state != (int)BrainFlowExitCodes::STATUS_OK)
            {
                {
                    std::lock_guard<std::mutex> lk (m);
                    state = (int)BrainFlowExitCodes::STATUS_OK;
                }
                cv.notify_one ();
            }
            push_package (data);
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
    delete[] data;
}

int SynchroniBoard::config_board (std::string config, std::string &response)
{
    if (dll_loader == NULL)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    int (*func) (void *) = (int (*) (void *))dll_loader->get_address ("synchroni_config_device");
    if (func == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for release");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }


    const int BUFFER_SIZE = 1024;
    char buffer[BUFFER_SIZE];
    memset (buffer, 0, sizeof (buffer));
    std::tuple<std::string, std::string, char *, int> info =
        std::make_tuple (params.mac_address, config, buffer, BUFFER_SIZE);

    int res = func ((void *)&info);
    response = buffer;
    return res;
}

int SynchroniBoard::call_init ()
{

    if (dll_loader == NULL)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    int (*func) (void *) = (int (*) (void *))dll_loader->get_address ("synchroni_initialize");
    if (func == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for initialize");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }

    std::tuple<int, struct BrainFlowInputParams, json> info =
        std::make_tuple (board_id, params, board_descr);

    int res = func ((void *)&info);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to initialize {}", res);
    }
    return res;
}

int SynchroniBoard::call_open ()
{
    if (dll_loader == NULL)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    int (*func) (void *) = (int (*) (void *))dll_loader->get_address ("synchroni_open_device");
    if (func == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for open_device");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    std::tuple<std::string> info = std::make_tuple (params.mac_address);
    return func ((void *)&info);
}

int SynchroniBoard::call_start ()
{
    if (dll_loader == NULL)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    int (*func) (void *) = (int (*) (void *))dll_loader->get_address ("synchroni_start_stream");
    if (func == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for start_stream");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    std::tuple<std::string> info = std::make_tuple (params.mac_address);
    return func ((void *)&info);
}

int SynchroniBoard::call_stop ()
{
    if (dll_loader == NULL)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    int (*func) (void *) = (int (*) (void *))dll_loader->get_address ("synchroni_stop_stream");
    if (func == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for stop_stream");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    std::tuple<std::string> info = std::make_tuple (params.mac_address);
    return func ((void *)&info);
}

int SynchroniBoard::call_close ()
{
    if (dll_loader == NULL)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    int (*func) (void *) = (int (*) (void *))dll_loader->get_address ("synchroni_close_device");
    if (func == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for close_device");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    std::tuple<std::string> info = std::make_tuple (params.mac_address);
    return func ((void *)&info);
}

int SynchroniBoard::call_release ()
{
    if (dll_loader == NULL)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    int (*func) (void *) = (int (*) (void *))dll_loader->get_address ("synchroni_release");
    if (func == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for release");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    std::tuple<std::string> info = std::make_tuple (params.mac_address);
    return func ((void *)&info);
}


int SynchroniBoard::prepare_session ()
{
    if (initialized)
    {
        safe_logger (spdlog::level::info, "Session is already prepared");
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    if (params.timeout <= 0)
    {
        params.timeout = 5;
    }
    if (!g_dll_loader)
    {
        g_dll_loader = make_shared<DLLLoader> (get_lib_name ().c_str ());
    }
    if (!dll_loader)
    {
        dll_loader = shared_ptr<DLLLoader> (g_dll_loader);
    }

    if (!dll_loader->load_library ())
    {
        safe_logger (spdlog::level::err, "Failed to load library");
        dll_loader = NULL;
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    safe_logger (spdlog::level::debug, "Library is loaded");
    int res = call_init ();
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        dll_loader = NULL;
        return res;
    }
    res = call_open ();
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        dll_loader = NULL;
        return res;
    }
    initialized = true;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int SynchroniBoard::start_stream (int buffer_size, const char *streamer_params)
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

int SynchroniBoard::stop_stream ()
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

int SynchroniBoard::release_session ()
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
        dll_loader = NULL;
    }

    return (int)BrainFlowExitCodes::STATUS_OK;
}
