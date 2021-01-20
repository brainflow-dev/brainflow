#include <chrono>
#include <stdlib.h>
#include <string.h>
#include <string>

#include "gforce_pro.h"

#ifdef _WIN32
#include <windows.h>

#include "custom_cast.h"
#include "get_dll_dir.h"

#include "gforce_wrapper_types.h"

int GforcePro::num_objects = 0;


GforcePro::GforcePro (struct BrainFlowInputParams params)
    : Board ((int)BoardIds::GFORCE_PRO_BOARD, params)
{
    GforcePro::num_objects++;
    if (GforcePro::num_objects > 1)
    {
        is_valid = false;
    }
    else
    {
        is_valid = true;
    }
    is_streaming = false;
    keep_alive = false;
    initialized = false;
    state = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;

    std::string gforcelib_path = "";
    std::string gforcelib_name = "";
    char gforcelib_dir[1024];
    bool res = get_dll_path (gforcelib_dir);
    if (sizeof (void *) == 4)
    {
        gforcelib_name = "gForceSDKWrapper32.dll";
    }
    else
    {
        gforcelib_name = "gForceSDKWrapper.dll";
    }


    if (res)
    {
        gforcelib_path = std::string (gforcelib_dir) + gforcelib_name;
        // gforcewrapper depends on gforcedll need to ensure that its in search path
        if (const char *env_p = std::getenv ("PATH"))
        {
            std::string path_env ("PATH=");
            path_env += env_p;
            path_env += ";";
            path_env += std::string (gforcelib_dir);
            if (_putenv (path_env.c_str ()) != 0)
            {
                safe_logger (spdlog::level::warn, "Failed to set PATH to {}", path_env.c_str ());
            }
        }
    }
    else
    {
        gforcelib_path = gforcelib_name;
    }

    safe_logger (spdlog::level::debug, "use dyn lib: {}", gforcelib_path.c_str ());
    dll_loader = new DLLLoader (gforcelib_path.c_str ());
}

GforcePro::~GforcePro ()
{
    skip_logs = true;
    GforcePro::num_objects--;
    release_session ();
}

int GforcePro::prepare_session ()
{
    if (initialized)
    {
        safe_logger (spdlog::level::info, "Session is already prepared");
        return (int)BrainFlowExitCodes::STATUS_OK;
    }

    if (!is_valid)
    {
        safe_logger (spdlog::level::info, "only one gforce per process is supported");
        return (int)BrainFlowExitCodes::ANOTHER_BOARD_IS_CREATED_ERROR;
    }

    if (!dll_loader->load_library ())
    {
        safe_logger (spdlog::level::err, "Failed to load library");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    safe_logger (spdlog::level::debug, "Library is loaded");

    int res = call_init ();
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }
    safe_logger (spdlog::level::debug, "gforce initialized");

    initialized = true;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int GforcePro::start_stream (int buffer_size, char *streamer_params)
{
    if (is_streaming)
    {
        safe_logger (spdlog::level::err, "Streaming thread already running");
        return (int)BrainFlowExitCodes::STREAM_ALREADY_RUN_ERROR;
    }

    int res = call_start ();
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }

    res = prepare_for_acquisition (buffer_size, streamer_params);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }

    keep_alive = true;
    streaming_thread = std::thread ([this] { read_thread (); });

    // wait for data to ensure that everything is okay
    std::unique_lock<std::mutex> lk (m);
    auto sec = std::chrono::seconds (1);
    if (cv.wait_for (
            lk, 7 * sec, [this] { return state != (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR; }))
    {
        is_streaming = true;
        return state;
    }
    else
    {
        safe_logger (spdlog::level::err, "no data received in {} sec, stopping thread", 7);
        is_streaming = true;
        stop_stream ();
        return (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
    }
}

int GforcePro::stop_stream ()
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

int GforcePro::release_session ()
{
    if (initialized)
    {
        stop_stream ();
        call_release ();
        initialized = false;
    }
    free_packages ();

    if (dll_loader != NULL)
    {
        dll_loader->free_library ();
        delete dll_loader;
        dll_loader = NULL;
    }

    return (int)BrainFlowExitCodes::STATUS_OK;
}

void GforcePro::read_thread ()
{
    int sleep_time = 2;

    int (*func) (void *) = (int (*) (void *))dll_loader->get_address ("gforceGetData");
    if (func == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for gforceGetData");
        return;
    }

    while (keep_alive)
    {
        struct GforceData data;
        int res = func ((void *)&data);
        if (res == (int)GforceWrapperExitCodes::STATUS_OK)
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
            Sleep (sleep_time);
        }
    }
}

int GforcePro::config_board (std::string config, std::string &response)
{
    safe_logger (spdlog::level::err, "Config board is not supported for Gforce PRO.");
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

int GforcePro::call_init ()
{
    if (dll_loader == NULL)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    int (*func) (void *) = (int (*) (void *))dll_loader->get_address ("gforceInitialize");
    if (func == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for gforceInitialize");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }

    int res = func (NULL);
    if (res != (int)GforceWrapperExitCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to init GForceWrapper {}", res);
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int GforcePro::call_start ()
{
    if (dll_loader == NULL)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    int (*func) (void *) = (int (*) (void *))dll_loader->get_address ("gforceStartStreaming");
    if (func == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for gforceStartStreaming");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }

    int res = func (NULL);
    if (res != (int)GforceWrapperExitCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to start streaming {}", res);
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int GforcePro::call_stop ()
{
    if (dll_loader == NULL)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    int (*func) (void *) = (int (*) (void *))dll_loader->get_address ("gforceStopStreaming");
    if (func == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for gforceStopStreaming");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }

    int res = func (NULL);
    if (res != (int)GforceWrapperExitCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to stop streaming {}", res);
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int GforcePro::call_release ()
{
    if (dll_loader == NULL)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    int (*func) (void *) = (int (*) (void *))dll_loader->get_address ("gforceRelease");
    if (func == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for gforceRelease");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }

    int res = func (NULL);
    if (res != (int)GforceWrapperExitCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to release GForceWrapper library {}", res);
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

#else

GforcePro::GforcePro (struct BrainFlowInputParams params)
    : Board ((int)BoardIds::GFORCE_PRO_BOARD, params)
{
}

GforcePro::~GforcePro ()
{
}

int GforcePro::prepare_session ()
{
    safe_logger (spdlog::level::err, "GforcePro doesnt support Unix.");
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

int GforcePro::config_board (std::string config, std::string &response)
{
    safe_logger (spdlog::level::err, "GforcePro doesnt support Unix.");
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

int GforcePro::release_session ()
{
    safe_logger (spdlog::level::err, "GforcePro doesnt support Unix.");
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

int GforcePro::stop_stream ()
{
    safe_logger (spdlog::level::err, "GforcePro doesnt support Unix.");
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

int GforcePro::start_stream (int buffer_size, char *streamer_params)
{
    safe_logger (spdlog::level::err, "GforcePro doesnt support Unix.");
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

#endif
