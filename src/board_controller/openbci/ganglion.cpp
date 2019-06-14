#include <string.h>

#include "GanglionNativeInterface.h"
#include "ganglion.h"

Ganglion::Ganglion (const char *port_name)
{
    this->num_channels = num_channels;
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
    is_streaming = false;
    keep_alive = false;
    initialized = false;
    db = NULL;
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

    if (!dll_loader->load_library ())
    {
        Board::board_logger->error ("failed to load library");
        return GENERAL_ERROR;
    }
    int res = STATUS_OK;
    if (this->use_mac_addr)
    {
        DLLFunc func = dll_loader->get_address ("open_ganglion_mac_addr_native");
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
        DLLFunc func = dll_loader->get_address ("open_ganglion_native");
        if (func == NULL)
        {
            Board::board_logger->error ("failed to get function address for open_ganglion_native");
            return GENERAL_ERROR;
        }
        res = (func) (NULL);
    }
    if (res != GanglionLibNative::CustomExitCodesNative::STATUS_OK)
    {
        Board::board_logger->error ("failed to Open Ganglion Device");
        return GENERAL_ERROR;
    }
    initialized = true;
    return STATUS_OK;
}

int Ganglion::start_stream (int buffer_size)
{
    if (is_streaming)
    {
        Board::board_logger->error ("Streaming thread already running");
        return STREAM_ALREADY_RUN_ERROR;
    }
    if (buffer_size <= 0 || buffer_size > MAX_CAPTURE_SAMPLES)
    {
        Board::board_logger->error ("invalid array size");
        return INVALID_BUFFER_SIZE_ERROR;
    }

    if (db)
    {
        delete db;
        db = NULL;
    }

    // start streaming
    DLLFunc func = dll_loader->get_address ("start_stream_native");
    if (func == NULL)
    {
        Board::board_logger->error ("failed to get function address for start_stream_native");
        return GENERAL_ERROR;
    }
    int res = (func) (NULL);
    if (res != GanglionLibNative::CustomExitCodesNative::STATUS_OK)
    {
        Board::board_logger->error ("failed to start streaming");
        return GENERAL_ERROR;
    }

    db = new DataBuffer (num_channels, buffer_size);
    if (!db->is_ready ())
    {
        Board::board_logger->error ("unable to prepare buffer");
        return INVALID_BUFFER_SIZE_ERROR;
    }

    keep_alive = true;
    streaming_thread = std::thread ([this] { this->read_thread (); });
    is_streaming = true;
    return STATUS_OK;
}

int Ganglion::stop_stream ()
{
    if (is_streaming)
    {
        keep_alive = false;
        is_streaming = false;
        streaming_thread.join ();
        DLLFunc func = dll_loader->get_address ("stop_stream_native");
        if (func == NULL)
        {
            Board::board_logger->error ("failed to get function address for stop_stream_native");
            return GENERAL_ERROR;
        }
        int res = (func) (NULL);
        if (res != GanglionLibNative::CustomExitCodesNative::STATUS_OK)
        {
            Board::board_logger->error ("failed to stop streaming");
            return GENERAL_ERROR;
        }
    }
    else
    {
        return STREAM_THREAD_IS_NOT_RUNNING;
    }
}

int Ganglion::release_session ()
{
    if (initialized)
    {
        if (is_streaming)
            stop_stream ();

        if (db)
        {
            delete db;
            db = NULL;
        }
        initialized = false;
    }
    DLLFunc func = dll_loader->get_address ("close_ganglion_native");
    if (func == NULL)
    {
        Board::board_logger->error ("failed to get function address for close_ganglion_native");
        return GENERAL_ERROR;
    }
    (func) (NULL);
    dll_loader->free_library ();
    delete dll_loader;
    dll_loader = NULL;
    return STATUS_OK;
}

void Ganglion::read_thread ()
{
    // https://docs.openbci.com/Hardware/08-Ganglion_Data_Format
    DLLFunc func = dll_loader->get_address ("get_data_native");
    while (keep_alive)
    {
        struct GanglionLibNative::GanglionDataNative data;
#ifdef _WIN32
        int res = (func) ((LPVOID)&data);
#endif
        if (res == GanglionLibNative::CustomExitCodesNative::STATUS_OK)
        {
            for (int i = 0; i < 20; i++)
            {
                Board::board_logger->trace ("package data {} {}", i, (int)data.data[i]);
            }
        }
        else
        {
            Board::board_logger->trace ("error {}", res);
            Sleep (10);
        }
    }
}
