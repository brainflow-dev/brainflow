#include <string>
#include <Windows.h>
#include <iostream>

#include "BrainAlive.h"
#include "custom_cast.h"
#include "get_dll_dir.h"
#include "timestamp.h"
#include "../BA_Native_Interface/BA_Native.h"

BrainAlive_Device::BrainAlive_Device (struct BrainFlowInputParams params)
    : Board ((int)BoardIds::BRAINALIVE_BOARD, params)
{
    char Brainalivelib_dir[1024];
    bool res = get_dll_path (Brainalivelib_dir);
    std::string brainalivelib_path = "";
#ifdef _WIN32
    std::string lib_name;
    if (sizeof (void *) == 4)
    {
        lib_name = "BA_Native_BLE.dll";
    }
    else
    {
        lib_name = "BA_Native_BLE.dll";
    }
#elif defined(__APPLE__)
    std::string lib_name = "libBrainFlowBluetooth.dylib";
#else
    std::string lib_name = "libBrainFlowBluetooth.so";
#endif
    if (res)
    {
        brainalivelib_path = std::string (Brainalivelib_dir) + lib_name;
    }
    else
    {
        brainalivelib_path = lib_name;
    }

    safe_logger (spdlog::level::debug, "use dyn lib: {}", brainalivelib_path.c_str ());
    dll_loader = new DLLLoader (brainalivelib_path.c_str ());

    keep_alive = false;
    initialized = false;
    func_get_data = NULL;
    state = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
}

BrainAlive_Device::~BrainAlive_Device ()
{
    skip_logs = true;
    release_session ();
}


int BrainAlive_Device::prepare_session ()
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

    func_get_data = (int (*) (void *))dll_loader->get_address ("get_data_native");

    if (func_get_data == NULL)
    {
        safe_logger (spdlog::level::err, "failed to find bluetooth_get_data");
        return_res = (int)BrainFlowExitCodes::GENERAL_ERROR;
    }

    if (params.mac_address.empty ())
    {
        safe_logger (
            spdlog::level::warn, "mac address is not provided, trying to autodiscover brainalive");
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
int BrainAlive_Device::start_stream (int buffer_size, char *streamer_params)
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
    safe_logger (spdlog::level::err, "failed to open bt connection: {}", res);
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
void BrainAlive_Device::read_thread ()
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
        int res = func_get_data (const_cast<char *> (params.mac_address.c_str ()));
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
            res = func_get_data (const_cast<char *> (params.mac_address.c_str ()));
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
                res = func_get_data (
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

int BrainAlive_Device::call_start ()
{
    int *mac_addr;
   /* int* (*func_open_device) (void *);

    func_open_device = ( int* (*) (void*))dll_loader->get_address ("open_brainalive_mac_addr_native");

    func_open_device ("1567");*/

    int (*func_open_device) (void*) =
        (int(*)(void*))dll_loader->get_address ("open_brainalive_mac_addr_native");
   
    if (func_open_device == NULL)
    {
        safe_logger (spdlog::level::err, "failed to find open brainalive funtion address");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }   
     safe_logger (spdlog::level::debug, "beforeconnect");
    
     int res = func_open_device (const_cast<char *> (params.mac_address.c_str ()));

   // printf (mac_addr);
    safe_logger (spdlog::level::debug, "after connect");
  if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to open bt connection: {}", res);
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
   /* int (*func_start_stream) (void *) = (int (*) (void *))dll_loader->get_address ("start_stream_native");
    if (func_start_stream == NULL)
    {
        safe_logger (spdlog::level::err, "failed to find start_stream_native funtion address");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    res = func_start_stream (NULL);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "Not able to write the command");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }*/
   // func_get_data = (int (*) (void *))dll_loader->get_address ("get_data_native");
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int BrainAlive_Device::call_stop ()
{
    int (*func_close) (void *) =
        (int (*) (void *))dll_loader->get_address ("close_brainalive_native");
    if (func_close == NULL)
    {
        safe_logger (
            spdlog::level::err, "failed to get function address for bluetooth_close_device");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    int res = func_close (const_cast<char *> (params.mac_address.c_str ()));
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to close bt connection: {}", res);
        return (int)BrainFlowExitCodes::BOARD_WRITE_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int BrainAlive_Device::stop_stream ()
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

int BrainAlive_Device::config_board (std::string config, std::string &response)
{
    safe_logger (spdlog::level::debug, "config_board is not supported for BrainALive");
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

int BrainAlive_Device::release_session ()
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
