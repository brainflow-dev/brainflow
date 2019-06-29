#pragma once

#ifdef _WIN32
#include <windows.h>
#define SHARED_EXPORT __declspec(dllexport)
#else
#define SHARED_EXPORT
#endif

#include <string.h>

namespace GanglionLibNative
{
#pragma pack(push, 1)
    struct GanglionDataNative
    {
        unsigned char data[20];
        long timestamp;
        GanglionDataNative (unsigned char *data, long timestamp)
        {
            memcpy (this->data, data, sizeof (unsigned char) * 20);
            this->timestamp = timestamp;
        }
        GanglionDataNative ()
        {
        }
        GanglionDataNative (const GanglionDataNative &other)
        {
            timestamp = other.timestamp;
            memcpy (data, other.data, sizeof (unsigned char) * 20);
        }
    };
#pragma pack(pop)

    enum CustomExitCodesNative
    {
        STATUS_OK = 0,
        GANGLION_NOT_FOUND_ERROR,
        GANGLION_IS_NOT_OPEN_ERROR,
        GANGLION_ALREADY_PAIR_ERROR,
        GANGLION_ALREADY_OPEN_ERROR,
        SERVICE_NOT_FOUND_ERROR,
        SEND_CHARACTERISTIC_NOT_FOUND_ERROR,
        RECEIVE_CHARACTERISTIC_NOT_FOUND_ERROR,
        DISCONNECT_CHARACTERISTIC_NOT_FOUND_ERROR,
        TIMEOUT_ERROR,
        STOP_ERROR,
        FAILED_TO_SET_CALLBACK_ERROR,
        FAILED_TO_UNSUBSCRIBE_ERROR,
        GENERAL_ERROR,
        NO_DATA_ERROR
    };

#ifdef __cplusplus
    extern "C"
    {
        SHARED_EXPORT int initialize (void *param);
        SHARED_EXPORT int open_ganglion_native (void *param);
        SHARED_EXPORT int open_ganglion_mac_addr_native (void *param);
        SHARED_EXPORT int stop_stream_native (void *param);
        SHARED_EXPORT int start_stream_native (void *param);
        SHARED_EXPORT int close_ganglion_native (void *param);
        SHARED_EXPORT int get_data_native (void *param);
    }
#endif
}
