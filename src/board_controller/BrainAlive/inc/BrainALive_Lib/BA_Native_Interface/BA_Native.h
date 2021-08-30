#pragma once
#ifdef _WIN32
#include <windows.h>
#define SHARED_EXPORT __declspec(dllexport)
#else
#define SHARED_EXPORT
#define BRAINALIVE_DONGLE_PORT "BRAINALIVE_DONGLE_PORT"
#endif
#include <string.h>

namespace BrainAliveNativeLib
{
#pragma pack(push, 1)
    struct BrainAliveDataNative
    {
        unsigned char data[20];
        long timestamp;
        BrainAliveDataNative (unsigned char *data, long timestamp)
        {
            memcpy (this->data, data, sizeof (unsigned char) * 20);
            this->timestamp = timestamp;
        }
        BrainAliveDataNative ()
        {
        }
        BrainAliveDataNative (const BrainAliveDataNative &other)
        {
            timestamp = other.timestamp;
            memcpy (data, other.data, sizeof (unsigned char) * 20);
        }
    };

    #pragma pack(pop)

    enum CustomExitCodesNative
    {
        STATUS_OK = 0,
        BRAINALIVE_NOT_FOUND_ERROR = 1,
        BRAINALIVE_IS_NOT_OPEN_ERROR = 2,
        BRAINALIVE_ALREADY_PAIR_ERROR = 3,
        BRAINALIVE_ALREADY_OPEN_ERROR = 4,
        SERVICE_NOT_FOUND_ERROR = 5,
        SEND_CHARACTERISTIC_NOT_FOUND_ERROR = 6,
        RECEIVE_CHARACTERISTIC_NOT_FOUND_ERROR = 7,
        DISCONNECT_CHARACTERISTIC_NOT_FOUND_ERROR = 8,
        TIMEOUT_ERROR = 9,
        STOP_ERROR = 10,
        FAILED_TO_SET_CALLBACK_ERROR = 11,
        FAILED_TO_UNSUBSCRIBE_ERROR = 12,
        GENERAL_ERROR = 13,
        NO_DATA_ERROR = 14,
        SYNC_ERROR = 15,
        NOT_IMPLEMENTED_ERROR = 16,
        INVALID_MAC_ADDR_ERROR = 17,
        PORT_OPEN_ERROR = 18
    };
#ifdef __cplusplus
    extern "C"
    {
        SHARED_EXPORT int initialize_native (void *param);
        SHARED_EXPORT int open_brainalive_native (void *param);
        SHARED_EXPORT int open_brainalive_mac_addr_native (void *param);
        SHARED_EXPORT int stop_stream_native (void *param);
        SHARED_EXPORT int start_stream_native (void *param);
        SHARED_EXPORT int close_brainalive_native (void *param);
        SHARED_EXPORT int get_data_native (void *param);
        SHARED_EXPORT int config_board_native (void *param);
        SHARED_EXPORT int release_native (void *param);
    }
#endif
}
