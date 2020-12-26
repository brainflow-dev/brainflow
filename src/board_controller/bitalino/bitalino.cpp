#include <chrono>
#include <string.h>
#include <string>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "custom_cast.h"
#include "bitalino.h"
#include "bitalino_types.h"
#include "get_dll_dir.h"
#include <iostream>


int BITalino::num_objects = 0;


BITalino::BITalino (struct BrainFlowInputParams params)
    : Board ((int)BoardIds::BITALINO_BOARD, params)
{
    BITalino::num_objects++;

    if (BITalino::num_objects > 1)
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
    num_channels = 6;
    state = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
    // Send command is depends on number of active channels and live or simulated mode
    // for example 22 is 22decimal value of 0x16 and it means simulated mode, one ecg channel
    //	and one 50Hz sinusoidal wave.
    // https://bitalino.com/datasheets/REVOLUTION_MCU_Block_Datasheet.pdf
    // Found the right command
    start_command = 13; //0x0d
    stop_command = "\0";

    std::string bitalinolib_path = "";
    std::string bitalinolib_name = "";
    char bitalinolib_dir[1024];
    bool res = get_dll_path (bitalinolib_dir);

#ifdef _WIN32
    if (sizeof (void *) == 4)
    {
        bitalinolib_name = "BITalinoLib32.dll";
    }
    else
    {
        bitalinolib_name = "BITalinoLib.dll";
    }
#endif
#ifdef __linux__
    bitalinolib_name = "libBITalinoLib.so";
#endif
#ifdef __APPLE__
    bitalinolib_name = "libBITalinoLib.dylib";
#endif

    if (res)
    {
        bitalinolib_path = std::string (bitalinolib_dir) + bitalinolib_name;
    }
    else
    {
        bitalinolib_path = bitalinolib_name;
    }

    safe_logger (spdlog::level::debug, "use dyn lib: {}", bitalinolib_path.c_str ());
    dll_loader = new DLLLoader (bitalinolib_path.c_str ());
}

BITalino::~BITalino ()
{
    skip_logs = true;
    BITalino::num_objects--;
    release_session ();
}

int BITalino::prepare_session ()
{
    if (initialized)
    {
        safe_logger (spdlog::level::info, "Session is already prepared");
        return (int)BrainFlowExitCodes::STATUS_OK;
    }

    if (!is_valid)
    {
        safe_logger (spdlog::level::info, "only one bitalino per process is supported");
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
    safe_logger (spdlog::level::debug, "bitalinolib initialized");
    res = call_open ();
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }
    initialized = true;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int BITalino::start_stream (int buffer_size, char *streamer_params)
{
    safe_logger (spdlog::level::err, "Streaming thread already running");
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
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }
    db = new DataBuffer (num_channels, buffer_size);
    if (!db->is_ready ())
    {
        Board::board_logger->error ("unable to prepare buffer with size {}", buffer_size);
        delete db;
        db = NULL;
        return (int)BrainFlowExitCodes::INVALID_BUFFER_SIZE_ERROR;
    }

    return start_streaming_prepared ();
}

int BITalino::start_streaming_prepared ()
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

int BITalino::stop_stream ()
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
        state = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
        return call_stop ();
    }
    else
    {
        return (int)BrainFlowExitCodes::STREAM_THREAD_IS_NOT_RUNNING;
    }
}

int BITalino::release_session ()
{
    if (initialized)
    {
        stop_stream ();
        initialized = false;
    }
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

void BITalino::read_thread ()
{
    int num_attempts = 0;
    int sleep_time = 10;
    int max_attempts = params.timeout * 1000 / sleep_time;
    double accel_x = 0.;
    double accel_y = 0.;
    double accel_z = 0.;
    double *mainpackage = new double[num_channels];
    unsigned long package[6];

    static const unsigned char CRC4tab[16] = {0, 3, 6, 5, 12, 15, 10, 9, 11, 8, 13, 14, 7, 4, 1, 2};

    int (*func) (void *) = (int (*) (void *))dll_loader->get_address ("get_data");
    if (func == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for get_data");
        return;
    }

    while (keep_alive)
    {
        for (int i = 0; i < num_channels; i++)
        {
            package[i] = 0.0;
            mainpackage[i] = 0.0;
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

            //_____________________________ BITalino data packet ______________________________
            // For BITalino data format use
            // https://bitalino.com/datasheets/REVOLUTION_MCU_Block_Datasheet.pdf

            int ptr = 0;
            for (int i = 0; i < 5; i++)
            {
                // CRC4 check function
                unsigned char crc = 0;
                unsigned char FinalCRC = 0;

                for (int ii = 0; ii < 3; ii++)
                {
                    const unsigned char b = data.data[ptr + ii];
                    crc = CRC4tab[crc] ^ (b >> 4);
                    crc = CRC4tab[crc] ^ (b & 0x0F);
                }
                // CRC for last byte
                crc = CRC4tab[crc] ^ (data.data[ptr + 3] >> 4);
                crc = CRC4tab[crc];

                FinalCRC = data.data[ptr + 3] & 0x0F;
                if (FinalCRC == crc)
                {
                    unsigned short d0 = data.data[ptr];
                    unsigned short d1 = data.data[ptr + 1];
                    unsigned short d2 = data.data[ptr + 2];
                    unsigned short d3 = data.data[ptr + 3];
                    ////////////////_______________________________________
                    package[1] = ((d2 & 0x000F) << 6) | (d1 >> 2); // A1 
                    package[2] = ((d1 & 0x0003) << 8) | (d0);      // A2 

					mainpackage[1] = uvolt * (eeg_scale * package[1] - eegMP);
					mainpackage[2] = uvolt * (eeg_scale * package[2] - eegMP);
						
                    mainpackage[3] = accel_x;
                    mainpackage[4] = accel_y;
                    mainpackage[5] = accel_z;

                    streamer->stream_data (mainpackage, num_channels, data.timestamp);
                    db->add_data (data.timestamp, mainpackage);
                    ptr += 4;
                }
                else
                {
                    ptr += 4;
                    continue;
                }
            }
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
    delete[] mainpackage;
}

int BITalino::config_board (std::string config, std::string &response)
{
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int BITalino::call_init ()
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
        safe_logger (spdlog::level::err, "failed to init BITalinoLib {}", res);
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int BITalino::call_open ()
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
        safe_logger (spdlog::level::err, "failed to Open Ganglion Device {}", res);
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int BITalino::call_config (char *config)
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

int BITalino::call_start ()
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

    int res = func ((void *)start_command.c_str ());
    if (res != (int)GanglionLib::CustomExitCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to start streaming {}", res);
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int BITalino::call_stop ()
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

int BITalino::call_close ()
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

int BITalino::call_release ()
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
