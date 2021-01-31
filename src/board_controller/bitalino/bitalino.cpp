#include <chrono>
#include <string.h>
#include <string>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "bitalino.h"
#include "bitalino_types.h"
#include "custom_cast.h"
#include "get_dll_dir.h"

int Bitalino::num_objects = 0;


Bitalino::Bitalino (struct BrainFlowInputParams params)
    : Board ((int)BoardIds::BITALINO_BOARD, params)
{
    Bitalino::num_objects++;

    if (Bitalino::num_objects > 1)
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
    // https://bitalino.com/storage/uploads/media/revolution-mcu-block-datasheet.pdf
    start_command = 253; // all channels on, live mode
    stop_command = "\0";

    std::string bitalinolib_path = "";
    std::string bitalinolib_name = "";
    char bitalinolib_dir[1024];
    bool res = get_dll_path (bitalinolib_dir);

#ifdef _WIN32
    if (sizeof (void *) == 4)
    {
        bitalinolib_name = "BitalinoLib32.dll";
    }
    else
    {
        bitalinolib_name = "BitalinoLib.dll";
    }
#endif
#ifdef __linux__
    bitalinolib_name = "libBitalinoLib.so";
#endif
#ifdef __APPLE__
    bitalinolib_name = "libBitalinoLib.dylib";
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

Bitalino::~Bitalino ()
{
    skip_logs = true;
    Bitalino::num_objects--;
    release_session ();
}

int Bitalino::prepare_session ()
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

int Bitalino::start_stream (int buffer_size, char *streamer_params)
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

int Bitalino::stop_stream ()
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

int Bitalino::release_session ()
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

void Bitalino::read_thread ()
{
    int num_attempts = 0;
    int sleep_time = 10;
    int max_attempts = params.timeout * 1000 / sleep_time;
    int num_rows = board_descr["num_rows"];
    double *package = new double[num_rows];
    for (int i = 0; i < num_channels; i++)
    {
        package[i] = 0.0;
    }

    int (*func) (void *) = (int (*) (void *))dll_loader->get_address ("get_data");
    if (func == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for get_data");
        return;
    }

    while (keep_alive)
    {
        struct BitalinoLib::BitalinoData data;
        int res = func ((void *)&data);
        if (res == (int)BitalinoLib::CustomExitCodes::STATUS_OK)
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

            //_____________________________ Bitalino data packet ______________________________
            // For Bitalino data format use
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

int Bitalino::config_board (std::string config, std::string &response)
{
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

int Bitalino::call_init ()
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

    struct BitalinoLib::BitalinoInputData input_data (params.timeout, params.serial_port.c_str ());
    int res = func ((void *)&input_data);
    if (res != (int)BitalinoLib::CustomExitCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to init BITalinoLib {}", res);
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int Bitalino::call_open ()
{
    if (dll_loader == NULL)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    int res = BitalinoLib::CustomExitCodes::GENERAL_ERROR;
    if (use_mac_addr)
    {
        int (*func) (void *) = (int (*) (void *))dll_loader->get_address ("open_bitalino_mac_addr");
        if (func == NULL)
        {
            safe_logger (
                spdlog::level::err, "failed to get function address for open_bitalino_mac_addr");
            return (int)BrainFlowExitCodes::GENERAL_ERROR;
        }

        safe_logger (spdlog::level::info, "search for {}", params.mac_address.c_str ());
        res = func (const_cast<char *> (params.mac_address.c_str ()));
    }
    else
    {
        int (*func) (void *) = (int (*) (void *))dll_loader->get_address ("open_bitalino");
        if (func == NULL)
        {
            safe_logger (spdlog::level::err, "failed to get function address for open_bitalino");
            return (int)BrainFlowExitCodes::GENERAL_ERROR;
        }

        safe_logger (
            spdlog::level::info, "mac address is not specified, try to find bitalino without it");
        res = func (NULL);
    }
    if (res != BitalinoLib::CustomExitCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to Open Bitalino Device {}", res);
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int Bitalino::call_start ()
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
    if (res != (int)BitalinoLib::CustomExitCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to start streaming {}", res);
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int Bitalino::call_stop ()
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
    if (res != (int)BitalinoLib::CustomExitCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to stop streaming {}", res);
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int Bitalino::call_close ()
{
    if (dll_loader == NULL)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    int (*func) (void *) = (int (*) (void *))dll_loader->get_address ("close_bitalino");
    if (func == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for close_bitalino");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }

    int res = func (NULL);
    if (res != (int)BitalinoLib::CustomExitCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to close bitalino {}", res);
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int Bitalino::call_release ()
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
    if (res != (int)BitalinoLib::CustomExitCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to release bitalino library {}", res);
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}
