#include <string.h>

#include "enophone.h"

#include "bluetooth_types.h"
#include "custom_cast.h"
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

    keep_alive = false;
    initialized = false;
    func_get_data = NULL;
    state = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
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
            spdlog::level::warn, "mac address is not provided, trying to autodiscover enophone");
        int res = find_enophone_addr ();
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

int Enophone::start_stream (int buffer_size, char *streamer_params)
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

    res = call_start ();
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

int Enophone::stop_stream ()
{
    if (keep_alive)
    {
        keep_alive = false;
        streaming_thread.join ();
        state = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
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
    int num_rows = board_descr["num_rows"];
    double *package = new double[num_rows];
    for (int i = 0; i < num_rows; i++)
    {
        package[i] = 0.0;
    }
    constexpr int buf_size = 20;
    char temp_buffer[buf_size];
    for (int i = 0; i < buf_size; i++)
    {
        temp_buffer[i] = 0;
    }

    while (keep_alive)
    {
        bool is_ready = false;
        // check first byte is 'b'
        int res = func_get_data (temp_buffer, 1, const_cast<char *> (params.mac_address.c_str ()));
        if ((res != 1) || (temp_buffer[0] != 'b'))
        {
            continue;
        }
        double timestamp = get_timestamp ();
        // notify main thread that 1st byte received
        if (state != (int)BrainFlowExitCodes::STATUS_OK)
        {
            {
                std::lock_guard<std::mutex> lk (m);
                state = (int)BrainFlowExitCodes::STATUS_OK;
            }
            cv.notify_one ();
            safe_logger (spdlog::level::debug, "start streaming");
        }

        // check second byte is 'S'
        while ((keep_alive) && (res >= 0))
        {
            res = func_get_data (
                temp_buffer + 1, 1, const_cast<char *> (params.mac_address.c_str ()));
            if (res == 1)
            {
                if (temp_buffer[1] == 'S')
                {
                    is_ready = true;
                }
                break;
            }
        }

        // first two bytes received, ready to read data bytes
        if (is_ready)
        {
            while ((keep_alive) && (res >= 0))
            {
                res = func_get_data (temp_buffer + 2, buf_size - 2,
                    const_cast<char *> (params.mac_address.c_str ()));
                if (res == buf_size - 2)
                {
                    std::vector<int> eeg_channels = board_descr["eeg_channels"];
                    for (int i = 0; i < 5; i++)
                    {
                        int32_t val = 0;
                        memcpy (&val, temp_buffer + i * sizeof (int32_t), sizeof (int32_t));
                        val = swap_endians (val);
                        if (i == 0)
                        {
                            package[board_descr["package_num_channel"].get<int> ()] = val % 65536;
                        }
                        else
                        {
                            double value = ((double)(val / 256)) * 5.0 / 8388608.0;
                            double gain = 100.0;
                            value = value / gain * 1000000.0;
                            package[eeg_channels[i - 1]] = value;
                        }
                    }
                    package[board_descr["timestamp_channel"].get<int> ()] = timestamp;
                    push_package (package);
                    break;
                }
            }
        }
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

    int res = func_open (1, const_cast<char *> (params.mac_address.c_str ()));
    if (res != (int)SocketBluetoothReturnCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to open bt connection: {}", res);
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
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
    int res = func_close (const_cast<char *> (params.mac_address.c_str ()));
    if (res != (int)SocketBluetoothReturnCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to close bt connection: {}", res);
        return (int)BrainFlowExitCodes::BOARD_WRITE_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int Enophone::find_enophone_addr ()
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
    std::string find_keyword = "nophone";
    int res = func_find (const_cast<char *> (find_keyword.c_str ()), mac_addr, &len);
    if (res == (int)SocketBluetoothReturnCodes::STATUS_OK)
    {
        std::string mac_string = mac_addr;
        params.mac_address = mac_string.substr (0, len);
    }

    return res;
}