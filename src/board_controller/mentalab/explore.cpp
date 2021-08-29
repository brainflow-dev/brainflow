#include <string.h>

#include "explore.h"

#include "bluetooth_types.h"
#include "custom_cast.h"
#include "get_dll_dir.h"
#include "timestamp.h"


Explore::Explore (int board_id, struct BrainFlowInputParams params) : Board (board_id, params)
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
#elif defined(__APPLE__)
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

    keep_alive = false;
    initialized = false;
    func_get_data = NULL;
    state = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
}

Explore::~Explore ()
{
    skip_logs = true;
    release_session ();
}

int Explore::prepare_session ()
{
    if (initialized)
    {
        safe_logger (spdlog::level::info, "Session is already prepared");
        return (int)BrainFlowExitCodes::STATUS_OK;
    }

    if (!dll_loader->load_library ())
    {
        safe_logger (spdlog::level::err, "Failed to load library");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    safe_logger (spdlog::level::debug, "Library is loaded");

    int return_res = (int)BrainFlowExitCodes::STATUS_OK;

    func_get_data = (int (*) (char *, int, char *))dll_loader->get_address ("bluetooth_get_data");
    if (func_get_data == NULL)
    {
        safe_logger (spdlog::level::err, "failed to find bluetooth_get_data");
        return_res = (int)BrainFlowExitCodes::GENERAL_ERROR;
    }

    if (params.mac_address.empty ())
    {
        safe_logger (
            spdlog::level::warn, "mac address is not provided, trying to autodiscover explore");
        int res = find_explore_addr ();
        if (res == (int)SocketBluetoothReturnCodes::UNIMPLEMENTED_ERROR)
        {
            safe_logger (spdlog::level::err, "autodiscovery for this OS is not supported");
            return_res = (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
        }
        else if (res == (int)SocketBluetoothReturnCodes::DEVICE_IS_NOT_CREATED_ERROR)
        {
            safe_logger (spdlog::level::err, "check that device paired and connected");
            return_res = (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
        }
        else if (res != (int)SocketBluetoothReturnCodes::STATUS_OK)
        {
            safe_logger (spdlog::level::err, "failed to autodiscover device: {}", res);
            return_res = (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
        else
        {
            safe_logger (spdlog::level::info, "found device {}", params.mac_address.c_str ());
        }
    }
    if (return_res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        return_res = call_open ();
    }
    if (return_res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        initialized = true;
    }
    else
    {
        dll_loader->free_library ();
        delete dll_loader;
        dll_loader = NULL;
    }

    return return_res;
}

int Explore::start_stream (int buffer_size, char *streamer_params)
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

    res = call_config (""); // todo command to start?
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

int Explore::stop_stream ()
{
    if (keep_alive)
    {
        keep_alive = false;
        streaming_thread.join ();
        state = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
        return call_config (""); // todo command to stop?
    }
    else
    {
        return (int)BrainFlowExitCodes::STREAM_THREAD_IS_NOT_RUNNING;
    }
}

int Explore::release_session ()
{
    if (initialized)
    {
        stop_stream ();
        call_close ();
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

void Explore::read_thread ()
{
    int num_rows = board_descr["num_rows"];
    double *package = new double[num_rows];
    for (int i = 0; i < num_rows; i++)
    {
        package[i] = 0.0;
    }

    while (keep_alive)
    {
        // todo parsing
    }
    delete[] package;
}

int Explore::config_board (std::string config, std::string &response)
{
    return call_config (config.c_str ());
}

int Explore::call_open ()
{
    int (*func_open) (int, char *) =
        (int (*) (int, char *))dll_loader->get_address ("bluetooth_open_device");
    if (func_open == NULL)
    {
        safe_logger (
            spdlog::level::err, "failed to get function address for bluetooth_open_device");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }

    int res = func_open (5, const_cast<char *> (params.mac_address.c_str ()));
    if (res != (int)SocketBluetoothReturnCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to open bt connection: {}", res);
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int Explore::call_close ()
{
    int (*func_close) (char *) =
        (int (*) (char *))dll_loader->get_address ("bluetooth_close_device");
    if (func_close == NULL)
    {
        safe_logger (
            spdlog::level::err, "failed to get function address for bluetooth_close_device");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    int res = func_close (const_cast<char *> (params.mac_address.c_str ()));
    if (res != (int)SocketBluetoothReturnCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to close bt connection: {}", res);
        return (int)BrainFlowExitCodes::BOARD_WRITE_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int Explore::call_config (const char *command)
{
    int (*func_config) (char *, int, char *) =
        (int (*) (char *, int, char *))dll_loader->get_address ("bluetooth_write_data");
    if (func_config == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for bluetooth_write_data");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }

    int res = func_config (const_cast<char *> (command), (int)strlen (command),
        const_cast<char *> (params.mac_address.c_str ()));
    if (res != (int)SocketBluetoothReturnCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to config board: {}", res);
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int Explore::find_explore_addr ()
{
    int (*func_find) (char *, char *, int *) =
        (int (*) (char *, char *, int *))dll_loader->get_address ("bluetooth_discover_device");
    if (func_find == NULL)
    {
        safe_logger (
            spdlog::level::err, "failed to get function address for bluetooth_discover_device");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    char mac_addr[40];
    int len = 0;
    std::string find_keyword = "Explore"; // todo check that
    int res = func_find (const_cast<char *> (find_keyword.c_str ()), mac_addr, &len);
    if (res == (int)SocketBluetoothReturnCodes::STATUS_OK)
    {
        std::string mac_string = mac_addr;
        params.mac_address = mac_string.substr (0, len);
    }

    return res;
}