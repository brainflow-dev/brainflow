#include <chrono>
#include <string.h>
#include <string>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "GanglionNativeInterface.h"
#include "custom_cast.h"
#include "ganglion.h"
#include "get_dll_dir.h"
#include "openbci_helpers.h"

// sleep is 10 ms, so wait for 2.5sec total
#define MAX_ATTEMPTS_TO_GET_DATA 250


Ganglion::Ganglion (const char *port_name) : Board ()
{
    if (port_name == NULL)
    {
        this->use_mac_addr = false;
    }
    else
    {
        strcpy (this->mac_addr, port_name);
        this->use_mac_addr = true;
    }
    // get full path of ganglioblibnative with assumption that this lib is in the same folder
    char ganglionlib_dir[1024];
    bool res = get_dll_path (ganglionlib_dir);
    std::string ganglioblib_path = "";
#ifdef _WIN32
    if (sizeof (void *) == 8)
    {
        if (res)
            ganglioblib_path = std::string (ganglionlib_dir) + "GanglionLibNative64.dll";
        else
            ganglioblib_path = "GanglionLibNative64.dll";
    }
    else
    {
        if (res)
            ganglioblib_path = std::string (ganglionlib_dir) + "GanglionLibNative32.dll";
        else
            ganglioblib_path = "GanglionLibNative32.dll";
    }
    safe_logger (spdlog::level::debug, "use dll: {}", ganglioblib_path.c_str ());
    dll_loader = new DLLLoader (ganglioblib_path.c_str ());
#else
    // temp unimplemented
    dll_loader = NULL;
#endif
    this->is_streaming = false;
    this->keep_alive = false;
    this->initialized = false;
    this->num_channels = 8;
    this->state = SYNC_TIMEOUT_ERROR;
}

Ganglion::~Ganglion ()
{
    skip_logs = true;
    release_session ();
}

int Ganglion::prepare_session ()
{
    if (initialized)
    {
        safe_logger (spdlog::level::info, "Session is already prepared");
        return STATUS_OK;
    }

    if (!this->dll_loader->load_library ())
    {
        safe_logger (spdlog::level::err, "failed to load library");
        return GENERAL_ERROR;
    }

    safe_logger (spdlog::level::debug, "Library is loaded");
    int res = this->call_init ();
    if (res != STATUS_OK)
    {
        return res;
    }
    safe_logger (spdlog::level::debug, "ganglionlib initialized");

    res = this->call_open ();
    if (res != STATUS_OK)
    {
        return res;
    }

    initialized = true;
    return STATUS_OK;
}

int Ganglion::start_stream (int buffer_size)
{
    if (this->is_streaming)
    {
        safe_logger (spdlog::level::err, "Streaming thread already running");
        return STREAM_ALREADY_RUN_ERROR;
    }
    if (buffer_size <= 0 || buffer_size > MAX_CAPTURE_SAMPLES)
    {
        safe_logger (spdlog::level::err, "invalid array size");
        return INVALID_BUFFER_SIZE_ERROR;
    }

    if (this->db)
    {
        delete this->db;
        this->db = NULL;
    }

    this->db = new DataBuffer (num_channels, buffer_size);
    if (!this->db->is_ready ())
    {
        Board::board_logger->error ("unable to prepare buffer with size {}", buffer_size);
        return INVALID_BUFFER_SIZE_ERROR;
    }

    int res = this->call_start ();
    if (res != STATUS_OK)
    {
        return res;
    }

    this->keep_alive = true;
    this->streaming_thread = std::thread ([this] { this->read_thread (); });

    // wait for data to ensure that everything is okay
    std::unique_lock<std::mutex> lk (this->m);
    auto sec = std::chrono::seconds (1);
    if (cv.wait_for (lk, 20 * sec, [this] { return this->state != SYNC_TIMEOUT_ERROR; }))
    {
        this->is_streaming = true;
        return this->state;
    }
    else
    {
        safe_logger (spdlog::level::err, "no data received in 20sec, stopping thread");
        this->is_streaming = true;
        this->stop_stream ();
        // return the same exit code as novaxr
        return UNABLE_TO_OPEN_PORT_ERROR;
    }
}

int Ganglion::stop_stream ()
{
    if (this->is_streaming)
    {
        this->keep_alive = false;
        this->is_streaming = false;
        this->streaming_thread.join ();
        this->state = SYNC_TIMEOUT_ERROR;
        return this->call_stop ();
    }
    else
    {
        return STREAM_THREAD_IS_NOT_RUNNING;
    }
}

int Ganglion::release_session ()
{
    if (this->initialized)
    {
        this->stop_stream ();

        if (this->db)
        {
            delete this->db;
            this->db = NULL;
        }
        this->initialized = false;
    }
    if (this->dll_loader != NULL)
    {
        this->call_close ();
        this->dll_loader->free_library ();
        delete this->dll_loader;
        this->dll_loader = NULL;
    }
    return STATUS_OK;
}

void Ganglion::read_thread ()
{
    // https://docs.openbci.com/Hardware/08-Ganglion_Data_Format
    DLLFunc func = dll_loader->get_address ("get_data_native");
    if (func == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for get_data_native");
        {
            std::lock_guard<std::mutex> lk (this->m);
            this->state = GENERAL_ERROR;
        }
        this->cv.notify_one ();
        return;
    }
    int num_attempts = 0;
    bool was_reset = false;

    float last_data[8] = {0};

    while (this->keep_alive)
    {
        struct GanglionLibNative::GanglionDataNative data;
        int res = (func) ((void *)&data);
        if (res == GanglionLibNative::CustomExitCodesNative::STATUS_OK)
        {
            if (this->state != STATUS_OK)
            {
                {
                    std::lock_guard<std::mutex> lk (this->m);
                    this->state = STATUS_OK;
                }
                this->cv.notify_one ();
                safe_logger (spdlog::level::debug, "start streaming");
            }

            float package[8] = {0.f};
            // delta holds 8 nums because (4 by each package)
            float delta[8] = {0.f};
            int bits_per_num = 0;
            unsigned char package_bits[160] = {0}; // 20 * 8
            for (int i = 0; i < 20; i++)
            {
                uchar_to_bits (data.data[i], package_bits + i * 8);
            }

            // no compression, used to init variable
            if (data.data[0] == 0)
            {
                // shift the last data packet to make room for a newer one
                last_data[0] = last_data[4];
                last_data[1] = last_data[5];
                last_data[2] = last_data[6];
                last_data[3] = last_data[7];

                // add new packet
                last_data[4] = cast_24bit_to_int32 (data.data + 1);
                last_data[5] = cast_24bit_to_int32 (data.data + 4);
                last_data[6] = cast_24bit_to_int32 (data.data + 7);
                last_data[7] = cast_24bit_to_int32 (data.data + 10);

                // scale new packet and insert into result
                package[0] = 0.f;
                package[1] = this->eeg_scale * last_data[4];
                package[2] = this->eeg_scale * last_data[5];
                package[3] = this->eeg_scale * last_data[6];
                package[4] = this->eeg_scale * last_data[7];

                // I dont understand how to get accel data, for now it's 0
                package[5] = 0.f;
                package[6] = 0.f;
                package[7] = 0.f;
                this->db->add_data (data.timestamp, package);
                continue;
            }
            // 18 bit compression, sends delta from previous value instead of real value!
            else if ((data.data[0] >= 1) && (data.data[0] <= 100))
            {
                bits_per_num = 18;
            }
            else if ((data.data[0] >= 101) && (data.data[0] <= 200))
            {
                bits_per_num = 19;
            }
            else if (data.data[0] > 200)
            {
                safe_logger (
                    spdlog::level::warn, "unexpected value {} in first byte", data.data[0]);
                continue;
            }
            // handle compressed data for 18 or 19 bits
            for (int i = 8, counter = 0; i < bits_per_num * 8; i += bits_per_num, counter++)
            {
                if (bits_per_num == 18)
                    delta[counter] = cast_ganglion_bits_to_int32<18> (package_bits + i);
                else
                    delta[counter] = cast_ganglion_bits_to_int32<19> (package_bits + i);
            }

            // apply the first delta to the last data we got in the previous iteration
            for (int i = 0; i < 4; i++)
            {
                last_data[i] = last_data[i + 4] - delta[i];
            }

            // apply the second delta to the previous packet which we just decompressed above
            for (int i = 4; i < 8; i++)
            {
                last_data[i] = last_data[i - 4] - delta[i];
            }

            // add first encoded package
            package[0] = data.data[0];
            package[1] = this->eeg_scale * last_data[0];
            package[2] = this->eeg_scale * last_data[1];
            package[3] = this->eeg_scale * last_data[2];
            package[4] = this->eeg_scale * last_data[3];
            this->db->add_data (data.timestamp, package);
            // add second package
            package[1] = this->eeg_scale * last_data[4];
            package[2] = this->eeg_scale * last_data[5];
            package[3] = this->eeg_scale * last_data[6];
            package[4] = this->eeg_scale * last_data[7];
            this->db->add_data (data.timestamp, package);
        }
        else
        {
            if (this->state == SYNC_TIMEOUT_ERROR)
            {
                num_attempts++;
            }
            if (num_attempts == MAX_ATTEMPTS_TO_GET_DATA)
            {
                if (was_reset)
                {
                    safe_logger (spdlog::level::err, "no data even after reset");
                    {
                        std::lock_guard<std::mutex> lk (this->m);
                        this->state = GENERAL_ERROR;
                    }
                    this->cv.notify_one ();
                    return;
                }
                else
                {
                    safe_logger (spdlog::level::warn, "resetting Ganglion device");
                    int tmp_res = this->call_close ();
                    if (tmp_res != STATUS_OK)
                    {
                        {
                            std::lock_guard<std::mutex> lk (this->m);
                            this->state = tmp_res;
                        }
                        this->cv.notify_one ();
                        return;
                    }
                    tmp_res = this->call_open ();
                    if (tmp_res != STATUS_OK)
                    {
                        {
                            std::lock_guard<std::mutex> lk (this->m);
                            this->state = tmp_res;
                        }
                        this->cv.notify_one ();
                        return;
                    }
                    tmp_res = this->call_start ();
                    if (tmp_res != STATUS_OK)
                    {
                        {
                            std::lock_guard<std::mutex> lk (this->m);
                            this->state = tmp_res;
                        }
                        this->cv.notify_one ();
                        return;
                    }
                    was_reset = true;
                }
            }
#ifdef _WIN32
            Sleep (10);
#else
            usleep (10000);
#endif
        }
    }
}

int Ganglion::config_board (char *config)
{
    safe_logger (spdlog::level::debug, "Trying to config Ganglion with {}", config);
    int res = validate_config (config);
    if (res != STATUS_OK)
    {
        return res;
    }
    return call_config (config);
}

int Ganglion::call_init ()
{
    DLLFunc func = this->dll_loader->get_address ("initialize_native");
    if (func == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for initialize");
        return GENERAL_ERROR;
    }
    int res = (func) (NULL);
    if (res != GanglionLibNative::CustomExitCodesNative::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to init GanglionLib {}", res);
        return GENERAL_ERROR;
    }
    return STATUS_OK;
}

int Ganglion::call_open ()
{
    int res = GanglionLibNative::CustomExitCodesNative::STATUS_OK;
    if (this->use_mac_addr)
    {
        safe_logger (spdlog::level::info, "search for {}", this->mac_addr);
        DLLFunc func = this->dll_loader->get_address ("open_ganglion_mac_addr_native");
        if (func == NULL)
        {
            safe_logger (spdlog::level::err,
                "failed to get function address for open_ganglion_mac_addr_native");
            return GENERAL_ERROR;
        }
        res = (func) (this->mac_addr);
    }
    else
    {
        safe_logger (
            spdlog::level::info, "mac address is not specified, try to find ganglion without it");
        DLLFunc func = this->dll_loader->get_address ("open_ganglion_native");
        if (func == NULL)
        {
            safe_logger (
                spdlog::level::err, "failed to get function address for open_ganglion_native");
            return GENERAL_ERROR;
        }
        res = (func) (NULL);
    }
    if (res != GanglionLibNative::CustomExitCodesNative::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to Open Ganglion Device {}", res);
        return GENERAL_ERROR;
    }
    safe_logger (spdlog::level::info, "Found Ganglion Device");
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
    if (res != GanglionLibNative::CustomExitCodesNative::STATUS_OK)
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
