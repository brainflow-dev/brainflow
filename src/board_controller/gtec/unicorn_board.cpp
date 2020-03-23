#include "unicorn_board.h"

// implementation for linux
#ifdef __linux__
#include "get_dll_dir.h"
#include "unicorn_board.h"
#include "unicorn_types.h"
#include <string.h>
#include <string>
#include <unistd.h>


UnicornBoard::UnicornBoard (struct BrainFlowInputParams params) : Board ((int)UNICORN_BOARD, params)
{
    // get full path of libunicorn.so with assumption that this lib is in the same folder
    char unicornlib_dir[1024];
    bool res = get_dll_path (unicornlib_dir);
    std::string unicornlib_path = "";
    if (res)
    {
        unicornlib_path = std::string (unicornlib_dir) + "libunicorn.so";
    }
    else
    {
        unicornlib_path = "libunicorn.so"
    }

    safe_logger (spdlog::level::debug, "use dyn lib: {}", unicornlib_path.c_str ());
    dll_loader = new DLLLoader (unicornlib_path.c_str ());

    is_streaming = false;
    keep_alive = false;
    initialized = false;
}

UnicornBoard::~UnicornBoard ()
{
    skip_logs = true;
    release_session ();
}

int UnicornBoard::prepare_session ()
{
    if (initialized)
    {
        safe_logger (spdlog::level::info, "Session is already prepared");
        return STATUS_OK;
    }
    // todo write smth to BrainFlowInputParams to select specific board for now choose the first one

    if (!dll_loader->load_library ())
    {
        safe_logger (spdlog::level::err, "Failed to load library");
        return GENERAL_ERROR;
    }
    safe_logger (spdlog::level::debug, "Library is loaded");

    int res = call_open ();
    if (res != STATUS_OK)
    {
        return res;
    }

    initialized = true;
    return STATUS_OK;
}

int UnicornBoard::start_stream (int buffer_size, char *streamer_params)
{
    if (is_streaming)
    {
        safe_logger (spdlog::level::err, "Streaming thread already running");
        return STREAM_ALREADY_RUN_ERROR;
    }
    if (buffer_size <= 0 || buffer_size > MAX_CAPTURE_SAMPLES)
    {
        safe_logger (spdlog::level::err, "Invalid array size");
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
        Board::board_logger->error ("Unable to prepare buffer with size {}", buffer_size);
        delete db;
        db = NULL;
        return INVALID_BUFFER_SIZE_ERROR;
    }

    int res = call_start ();
    if (res != STATUS_OK)
    {
        return res;
    }

    keep_alive = true;
    streaming_thread = std::thread ([this] { this->read_thread (); });
    is_streaming = true;
    return STATUS_OK;
}

int UnicornBoard::stop_stream ()
{
    if (is_streaming)
    {
        keep_alive = false;
        is_streaming = false;
        streaming_thread.join ();
        return call_stop ();
    }
    else
    {
        return STREAM_THREAD_IS_NOT_RUNNING;
    }
}

int UnicornBoard::release_session ()
{
    if (initialized)
    {
        stop_stream ();

        if (db)
        {
            delete db;
            db = NULL;
        }
        initialized = false;
    }
    if (dll_loader != NULL)
    {
        call_close ();
        dll_loader->free_library ();
        delete dll_loader;
        dll_loader = NULL;
    }
    return STATUS_OK;
}

void UnicornBoard::read_thread ()
{
    while (this->keep_alive)
    {
    }
}

int UnicornBoard::config_board (char *config)
{
    safe_logger (spdlog::level::debug, "config_board in todo list for Unicorn.");
    return UNSUPPORTED_BOARD_ERROR;
}

int UnicornBoard::call_open ()
{
    int (*func) (UNICORN_DEVICE_SERIAL *, uint32_t *, BOOL) =
        dll_loader->get_address ("UNICORN_GetAvailableDevices");
    if (func == NULL)
    {
        safe_logger (
            spdlog::level::err, "failed to get function address for UNICORN_GetAvailableDevices");
        return GENERAL_ERROR;
    }
    unsigned int available_device_count = 0;
    int ec = func (NULL, &available_device_count, TRUE);
    if (ec != UNICORN_ERROR_SUCCESS)
    {
        safe_logger (spdlog::level::err, "Error in UNICORN_GetAvailableDevices {}", ec);
        return BOARD_NOT_READY_ERROR;
    }
    if (available_device_count < 1)
    {
        safe_logger (spdlog::level::err, "Unicorn not found");
        return BOARD_NOT_READY_ERROR;
    }


    return STATUS_OK;
}

int Ganglion::call_open ()
{
    int res = GanglionLibNative::CustomExitCodesNative::GENERAL_ERROR;
#ifdef _WIN32
    int num_attempts = 2;
#else
    int num_attempts = 1;
#endif
    // its bad but we need to make it as reliable as possible, once per ~200 runs it fails to
    // open ganglion and I can not fix it in C# code(wo restart) so lets restart it here,
    // bigger delay is better than failure!
    for (int i = 0; i < num_attempts; i++)
    {
        safe_logger (spdlog::level::debug, "trying to open ganglion {}/{}", i, num_attempts);
        if (this->use_mac_addr)
        {
            safe_logger (spdlog::level::info, "search for {}", this->params.mac_address.c_str ());
            DLLFunc func = this->dll_loader->get_address ("open_ganglion_mac_addr_native");
            if (func == NULL)
            {
                safe_logger (spdlog::level::err,
                    "failed to get function address for open_ganglion_mac_addr_native");
                return GENERAL_ERROR;
            }
            res = (func) (const_cast<char *> (params.mac_address.c_str ()));
        }
        else
        {
            safe_logger (spdlog::level::info,
                "mac address is not specified, try to find ganglion without it");
            DLLFunc func = this->dll_loader->get_address ("open_ganglion_native");
            if (func == NULL)
            {
                safe_logger (
                    spdlog::level::err, "failed to get function address for open_ganglion_native");
                return GENERAL_ERROR;
            }
            res = (func) (NULL);
        }
        if (res == GanglionLibNative::CustomExitCodesNative::STATUS_OK)
        {
            safe_logger (spdlog::level::info, "Ganglion is opened and paired");
            break;
        }
        // sanity check, e.g. device was found but services were not found
        call_close ();
    }
    if (res != GanglionLibNative::CustomExitCodesNative::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to Open Ganglion Device {}", res);
        return GENERAL_ERROR;
    }
    return STATUS_OK;
}

int Ganglion::call_config (char *config)
{
    DLLFunc func = this->dll_loader->get_address ("config_board_native");
    if (func == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for config_board_native");
        return GENERAL_ERROR;
    }
    int res = (func) (config);
    if (res != GanglionLibNative::CustomExitCodesNative::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to config board {}", res);
        return GENERAL_ERROR;
    }
    return STATUS_OK;
}

int Ganglion::call_start ()
{
    DLLFunc func = this->dll_loader->get_address ("start_stream_native");
    if (func == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for start_stream_native");
        return GENERAL_ERROR;
    }
    int res = (func) (NULL);
    if (res != (int)GanglionLibNative::CustomExitCodesNative::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to start streaming {}", res);
        return GENERAL_ERROR;
    }
    return STATUS_OK;
}

int Ganglion::call_stop ()
{
    DLLFunc func = dll_loader->get_address ("stop_stream_native");
    if (func == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for stop_stream_native");
        return GENERAL_ERROR;
    }
    int res = (func) (NULL);
    if (res != GanglionLibNative::CustomExitCodesNative::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to stop streaming {}", res);
        return GENERAL_ERROR;
    }
    return STATUS_OK;
}

int Ganglion::call_close ()
{
    DLLFunc func = dll_loader->get_address ("close_ganglion_native");
    if (func == NULL)
    {
        safe_logger (
            spdlog::level::err, "failed to get function address for close_ganglion_native");
        return GENERAL_ERROR;
    }
    int res = (func) (NULL);
    if (res != GanglionLibNative::CustomExitCodesNative::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to close Ganglion {}", res);
        return GENERAL_ERROR;
    }
    return STATUS_OK;
}

int Ganglion::call_release ()
{
    DLLFunc func = this->dll_loader->get_address ("release_native");
    if (func == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for release_native");
        return GENERAL_ERROR;
    }
    int res = (func) (NULL);
    if (res != GanglionLibNative::CustomExitCodesNative::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to release ganglion {}", res);
        return GENERAL_ERROR;
    }
    return STATUS_OK;
}

// stub for windows and macos
#else
UnicornBoard::UnicornBoard (struct BrainFlowInputParams params) : Board ((int)UNICORN_BOARD, params)
{
}

UnicornBoard::~UnicornBoard ()
{
}

int UnicornBoard::prepare_session ()
{
    safe_logger (spdlog::level::err, "UnicornBoard supports only Linux.");
    return UNSUPPORTED_BOARD_ERROR;
}

int UnicornBoard::config_board (char *config)
{
    safe_logger (spdlog::level::err, "UnicornBoard supports only Linux.");
    return UNSUPPORTED_BOARD_ERROR;
}

int UnicornBoard::release_session ()
{
    safe_logger (spdlog::level::err, "UnicornBoard supports only Linux.");
    return UNSUPPORTED_BOARD_ERROR;
}

int UnicornBoard::stop_stream ()
{
    safe_logger (spdlog::level::err, "UnicornBoard supports only Linux.");
    return UNSUPPORTED_BOARD_ERROR;
}

int UnicornBoard::start_stream (int buffer_size, char *streamer_params)
{
    safe_logger (spdlog::level::err, "UnicornBoard supports only Linux.");
    return UNSUPPORTED_BOARD_ERROR;
}
#endif
