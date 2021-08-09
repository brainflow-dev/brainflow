#include <string.h>

#include "enophone.h"
#include "get_dll_dir.h"
#include "timestamp.h"


Enophone::Enophone (struct BrainFlowInputParams params)
    : Board ((int)BoardIds::ENOPHONE_BOARD, params)
{
    char bluetoothlib_dir[1024];
    bool res = get_dll_path (bluetoothlib_dir);
    std::string bluetoothlib_path = "";
#ifdef _WIN32
    std::string lib_name;
    if (sizeof (void *) == 4)
    {
        lib_name = "BrainFlowBluetooth32.dll";
    }
    else
    {
        lib_name = "BrainFlowBluetooth.dll";
    }
#elif defined(__apple__)
    std::string lib_name = "libBrainFlowBluetooth.dylib";
#else
    std::string lib_name = "libBrainFlowBluetooth.so";
#endif
    if (res)
    {
        bluetoothlib_path = std::string (bluetoothlib_dir) + lib_name;
    }
    else
    {
        bluetoothlib_path = lib_name;
    }

    safe_logger (spdlog::level::debug, "use dyn lib: {}", bluetoothlib_path.c_str ());
    dll_loader = new DLLLoader (bluetoothlib_path.c_str ());

    is_streaming = false;
    keep_alive = false;
    initialized = false;
}

Enophone::~Enophone ()
{
    skip_logs = true;
    release_session ();
}

int Enophone::prepare_session ()
{
    if (initialized)
    {
        safe_logger (spdlog::level::info, "Session is already prepared");
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    if (params.mac_address.empty ())
    {
        safe_logger (spdlog::level::err, "mac address is not provided");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    if (!dll_loader->load_library ())
    {
        safe_logger (spdlog::level::err, "Failed to load library");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    safe_logger (spdlog::level::debug, "Library is loaded");

    initialized = true;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int Enophone::start_stream (int buffer_size, char *streamer_params)
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

    res = call_start ();
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }

    keep_alive = true;
    streaming_thread = std::thread ([this] { this->read_thread (); });
    is_streaming = true;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int Enophone::stop_stream ()
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
        return (int)BrainFlowExitCodes::STREAM_THREAD_IS_NOT_RUNNING;
    }
}

int Enophone::release_session ()
{
    if (initialized)
    {
        stop_stream ();
        free_packages ();
        initialized = false;
    }
    if (dll_loader != NULL)
    {
        dll_loader->free_library ();
        delete dll_loader;
        dll_loader = NULL;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

void Enophone::read_thread ()
{
    int (*func_get_data) (char *, int, char *);
    func_get_data = (int (*) (char *, int, char *))dll_loader->get_address ("bluetooth_get_data");
    int num_rows = board_descr["num_rows"];
    double *package = new double[num_rows];
    for (int i = 0; i < num_rows; i++)
    {
        package[i] = 0.0;
    }

    constexpr int buf_size = 50;
    char temp_buffer[buf_size];

    while (keep_alive)
    {
        int res =
            func_get_data (temp_buffer, buf_size, const_cast<char *> (params.mac_address.c_str ()));
        if ((res != buf_size) && (res != 0))
        {
            safe_logger (spdlog::level::warn, "read {} bytes", buf_size, res);
            continue;
        }
        package[board_descr["timestamp_channel"].get<int> ()] = get_timestamp ();
        push_package (package);
    }
    delete[] package;
}

int Enophone::config_board (std::string config, std::string &response)
{
    safe_logger (spdlog::level::debug, "config_board is not supported for Enophone");
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

int Enophone::call_start ()
{
    int (*func_open) (int, char *) =
        (int (*) (int, char *))dll_loader->get_address ("bluetooth_open_device");
    if (func_open == NULL)
    {
        safe_logger (
            spdlog::level::err, "failed to get function address for bluetooth_open_device");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }

    if (params.mac_address.empty ())
    {
        safe_logger (spdlog::level::err, "mac address is not provided.");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    return func_open (1, const_cast<char *> (params.mac_address.c_str ()));
}

int Enophone::call_stop ()
{
    int (*func_close) (char *) =
        (int (*) (char *))dll_loader->get_address ("bluetooth_close_device");
    if (func_close == NULL)
    {
        safe_logger (
            spdlog::level::err, "failed to get function address for bluetooth_close_device");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    return func_close (const_cast<char *> (params.mac_address.c_str ()));
}
