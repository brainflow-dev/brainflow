#include <iostream>
#include <queue>
#include <stdlib.h>
#include <thread>

#ifdef _WIN32
#include <windows.h>
#define FILETIME_TO_UNIX 116444736000000000i64
#else
#include <sys/time.h>
#include <unistd.h>
#endif

#include "GanglionNativeInterface.h"

#ifdef _WIN32
double get_timestamp ()
{
    FILETIME ft;
    GetSystemTimePreciseAsFileTime (&ft);
    int64_t t = ((int64_t)ft.dwHighDateTime << 32L) | (int64_t)ft.dwLowDateTime;
    return (t - FILETIME_TO_UNIX) / (10.0 * 1000.0 * 1000.0);
}
#else
double get_timestamp ()
{
    struct timeval tv;
    gettimeofday (&tv, NULL);
    return (double)(tv.tv_sec) + (double)(tv.tv_usec) / 1000000.0;
}
#endif

// code is messy and full of warnings but its just a mock for CI
namespace GanglionLibNative
{
    bool is_open = false;
    std::thread worker_thread;
    bool thread_should_stop = false;
    std::queue<struct GanglionDataNative> data_queue;

    void thread_func ();

    int initialize_native (void *param)
    {
        std::cout << "initialized" << std::endl;
        return STATUS_OK;
    }

    int open_ganglion_native (void *param)
    {
        if (is_open)
        {
            std::cout << "already open" << std::endl;
            return GANGLION_ALREADY_OPEN_ERROR;
        }
        is_open = true;
        return STATUS_OK;
    }

    int open_ganglion_mac_addr_native (void *param)
    {
        if (is_open)
        {
            std::cout << "already open" << std::endl;
            return GANGLION_ALREADY_OPEN_ERROR;
        }
        if (param == NULL)
        {
            std::cout << "invalid param" << std::endl;
            return GENERAL_ERROR;
        }
        is_open = true;
        return STATUS_OK;
    }

    int start_stream_native (void *param)
    {
        if (!is_open)
        {
            std::cout << "is not open" << std::endl;
            return GANGLION_IS_NOT_OPEN_ERROR;
        }
        if (worker_thread.joinable ())
        {
            std::cout << "streaming is already running" << std::endl;
            return GENERAL_ERROR;
        }
        thread_should_stop = false;
        worker_thread = std::thread (thread_func);
        return STATUS_OK;
    }

    int stop_stream_native (void *param)
    {
        if (!worker_thread.joinable ())
        {
            std::cout << "streaming is not running" << std::endl;
            return STOP_ERROR;
        }
        thread_should_stop = true;
        worker_thread.join ();
        return STATUS_OK;
    }

    int close_ganglion_native (void *param)
    {
        stop_stream_native (NULL);
        is_open = false;
        return STATUS_OK;
    }

    int config_board_native (void *param)
    {
        return STATUS_OK;
    }

    void thread_func ()
    {
        unsigned int counter = 0;
        unsigned char values[20];
        while (!thread_should_stop)
        {
            if (counter % 100)
            {
                values[0] = 0;
            }
            else
            {
                values[0] = (unsigned char)(((double)rand () / (double)RAND_MAX) * 200);
            }
            for (int i = 1; i < 20; i++)
            {
                values[i] = (unsigned char)(((double)rand () / (double)RAND_MAX) * 255);
            }
            struct GanglionDataNative data (values, (long)get_timestamp ());
            data_queue.push (data);
#ifdef _WIN32
            Sleep (10);
#else
            usleep (10000)
#endif
        }
    }

    int get_data_native (void *param)
    {
        if (data_queue.empty ())
        {
            return NO_DATA_ERROR;
        }
        struct GanglionDataNative &data = data_queue.front ();
        struct GanglionDataNative *out_data = (struct GanglionDataNative *)(param);
        out_data->timestamp = data.timestamp;
        for (int i = 0; i < 20; i++)
        {
            out_data->data[i] = data.data[i];
        }
        data_queue.pop ();

        return STATUS_OK;
    }
}