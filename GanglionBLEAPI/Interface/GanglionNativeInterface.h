#pragma once

#ifdef _WIN32
#include <Windows.h>
#endif

namespace GanglionLibNative
{
#pragma pack(push, 1)
    struct GanglionDataNative
    {
        unsigned char data[20];
        long timestamp;
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
#ifdef _WIN32
        __declspec(dllexport) int __cdecl open_ganglion_native (LPVOID param);
        __declspec(dllexport) int __cdecl open_ganglion_mac_addr_native (LPVOID param);
        __declspec(dllexport) int __cdecl stop_stream_native (LPVOID param);
        __declspec(dllexport) int __cdecl start_stream_native (LPVOID param);
        __declspec(dllexport) int __cdecl close_ganglion_native (LPVOID param);
        __declspec(dllexport) int __cdecl get_data_native (LPVOID param);
#endif
    }
#endif
}