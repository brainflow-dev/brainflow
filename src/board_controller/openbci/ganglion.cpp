#include <chrono>
#include <string.h>

#ifdef _WIN32
#include <Windows.h>
#endif

#include "GanglionNativeInterface.h"
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