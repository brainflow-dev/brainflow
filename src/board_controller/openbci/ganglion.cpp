#include <chrono>
#include <string.h>

#ifdef _WIN32
#include <Windows.h>
#endif

#include "GanglionNativeInterface.h"
#include "custom_cast.h"
#include "ganglion.h"

// sleep is 10 ms, so wait for 1sec total
#define MAX_ATTEMPTS_TO_GET_DATA 100


Ganglion::Ganglion (const char *port_name)
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
#ifdef _WIN32
    if (sizeof (void *) == 8)
    {
        const char *dll_name = "GanglionLibNative64.dll";
        Board::board_logger->debug ("use dll: {}", dll_name);
        dll_loader = new DLLLoader (dll_name);
    }
    else
    {
        const char *dll_name = "GanglionLibNative32.dll";
        Board::board_logger->debug ("use dll: {}", dll_name);
        dll_loader = new DLLLoader (dll_name);
    }
#else
#endif
    this->is_streaming = false;
    this->keep_alive = false;
    this->initialized = false;
    this->db = NULL;
    this->num_channels = 8;
    this->state = SYNC_TIMEOUT_ERROR;
}

Ganglion::~Ganglion ()
{
    release_session ();
}

int Ganglion::prepare_session ()
{
    if (initialized)
    {
        Board::board_logger->info ("Session is already prepared");
        return STATUS_OK;
    }

    if (!this->dll_loader->load_library ())
    {
        Board::board_logger->error ("failed to load library");
        return GENERAL_ERROR;
    }

    Board::board_logger->debug ("Library is loaded");
    int res = this->call_init ();
    if (res != STATUS_OK)
    {
        return res;
    }
    Board::board_logger->debug ("ganglionlib initialized");

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
        Board::board_logger->error ("Streaming thread already running");
        return STREAM_ALREADY_RUN_ERROR;
    }
    if (buffer_size <= 0 || buffer_size > MAX_CAPTURE_SAMPLES)
    {
        Board::board_logger->error ("invalid array size");
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
        Board::board_logger->error ("no data received in 20sec, stopping thread");
        this->is_streaming = true;
        this->stop_stream ();
        return this->state;
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
        Board::board_logger->error ("failed to get function address for get_data_native");
        {
            std::lock_guard<std::mutex> lk (this->m);
            this->state = GENERAL_ERROR;
        }
        this->cv.notify_one ();
        return;
    }
    int num_attempts = 0;
    bool was_reset = false;
    while (this->keep_alive)
    {
        struct GanglionLibNative::GanglionDataNative data;
#ifdef _WIN32
        int res = (func) ((LPVOID)&data);
#endif
        if (res == GanglionLibNative::CustomExitCodesNative::STATUS_OK)
        {
            if (this->state != STATUS_OK)
            {
                {
                    std::lock_guard<std::mutex> lk (this->m);
                    this->state = STATUS_OK;
                }
                this->cv.notify_one ();
                Board::board_logger->debug ("start streaming");
            }

            float package[8] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
            // last_data and delta hold 8 nums because (4 by each package)
            float last_data[8] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
            float delta[8] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
            int bytes_per_num = 0;
            unsigned char package_bytes[160]; // 20 * 8
            for (int i = 0; i < 20; i++)
            {
                uchar_to_bits (data.data[i], package_bytes + i * 8);
            }
            // no compression, used to init variable
            if (data.data[0] == 0)
            {
                last_data[0] = cast_24bit_to_int32 (data.data + 1);
                last_data[1] = cast_24bit_to_int32 (data.data + 4);
                last_data[2] = cast_24bit_to_int32 (data.data + 7);
                last_data[3] = cast_24bit_to_int32 (data.data + 10);
                last_data[4] = last_data[0];
                last_data[5] = last_data[1];
                last_data[6] = last_data[2];
                last_data[7] = last_data[3];

                package[0] = 0.f;
                package[1] = this->eeg_scale * last_data[0];
                package[2] = this->eeg_scale * last_data[1];
                package[3] = this->eeg_scale * last_data[2];
                package[4] = this->eeg_scale * last_data[3];

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
                bytes_per_num = 18;
            }
            else if ((data.data[0] >= 101) && (data.data[0] <= 200))
            {
                bytes_per_num = 19;
            }
            else if (data.data[0] > 200)
            {
                Board::board_logger->warn ("unxpected value {} in first byte", data.data[0]);
                continue;
            }
            // handle compressed data for 18 or 19 bits
            for (int i = 8, counter = 0; i < bytes_per_num * 8; i += bytes_per_num, counter++)
            {
                if (bytes_per_num == 18)
                    delta[counter] = cast_18bit_to_int32 (package_bytes + i);
                else
                    delta[counter] = cast_19bit_to_int32 (package_bytes + i);
                last_data[counter] -= delta[counter];
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
                    Board::board_logger->error ("no data even after reset");
                    {
                        std::lock_guard<std::mutex> lk (this->m);
                        this->state = GENERAL_ERROR;
                    }
                    this->cv.notify_one ();
                    return;
                }
                else
                {
                    Board::board_logger->warn ("resetting Ganglion device");
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
#endif
        }
    }
}

int Ganglion::call_start ()
{
    DLLFunc func = this->dll_loader->get_address ("start_stream_native");
    if (func == NULL)
    {
        Board::board_logger->error ("failed to get function address for start_stream_native");
        return GENERAL_ERROR;
    }
    int res = (func) (NULL);
    if (res != GanglionLibNative::CustomExitCodesNative::STATUS_OK)
    {
        Board::board_logger->error ("failed to start streaming {}", res);
        return GENERAL_ERROR;
    }
    return STATUS_OK;
}

int Ganglion::call_init ()
{
    DLLFunc func = this->dll_loader->get_address ("initialize");
    if (func == NULL)
    {
        Board::board_logger->error ("failed to get function address for initialize");
        return GENERAL_ERROR;
    }
    int res = (func) (NULL);
    if (res != GanglionLibNative::CustomExitCodesNative::STATUS_OK)
    {
        Board::board_logger->error ("failed to init GanglionLib {}", res);
        return GENERAL_ERROR;
    }
    return STATUS_OK;
}

int Ganglion::call_open ()
{
    int res = GanglionLibNative::CustomExitCodesNative::STATUS_OK;
    if (this->use_mac_addr)
    {
        Board::board_logger->info ("search for {}", this->mac_addr);
        DLLFunc func = this->dll_loader->get_address ("open_ganglion_mac_addr_native");
        if (func == NULL)
        {
            Board::board_logger->error (
                "failed to get function address for open_ganglion_mac_addr_native");
            return GENERAL_ERROR;
        }
        res = (func) (this->mac_addr);
    }
    else
    {
        Board::board_logger->warn ("mac address is not specified, try to find ganglion without it");
        DLLFunc func = this->dll_loader->get_address ("open_ganglion_native");
        if (func == NULL)
        {
            Board::board_logger->error ("failed to get function address for open_ganglion_native");
            return GENERAL_ERROR;
        }
        res = (func) (NULL);
    }
    if (res != GanglionLibNative::CustomExitCodesNative::STATUS_OK)
    {
        Board::board_logger->error ("failed to Open Ganglion Device {}", res);
        return GENERAL_ERROR;
    }
    Board::board_logger->info ("Found Ganglion Device");
    return STATUS_OK;
}

int Ganglion::call_stop ()
{
    DLLFunc func = dll_loader->get_address ("stop_stream_native");
    if (func == NULL)
    {
        Board::board_logger->error ("failed to get function address for stop_stream_native");
        return GENERAL_ERROR;
    }
    int res = (func) (NULL);
    if (res != GanglionLibNative::CustomExitCodesNative::STATUS_OK)
    {
        Board::board_logger->error ("failed to stop streaming {}", res);
        return GENERAL_ERROR;
    }
    return STATUS_OK;
}

int Ganglion::call_close ()
{
    DLLFunc func = dll_loader->get_address ("close_ganglion_native");
    if (func == NULL)
    {
        Board::board_logger->error ("failed to get function address for close_ganglion_native");
        return GENERAL_ERROR;
    }
    int res = (func) (NULL);
    if (res != GanglionLibNative::CustomExitCodesNative::STATUS_OK)
    {
        Board::board_logger->error ("failed to close Ganglion {}", res);
        return GENERAL_ERROR;
    }
    return STATUS_OK;
}