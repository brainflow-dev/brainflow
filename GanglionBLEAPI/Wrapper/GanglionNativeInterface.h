#pragma once


namespace GanglionLibNative
{
    #pragma pack (push, 1)
    struct GanglionDataNative
    {
        unsigned char data[20];
        long timestamp;
    };
    #pragma pack (pop)

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
        __declspec(dllexport) int open_ganglion_native ();
        __declspec(dllexport) int open_ganglion_mac_addr_native (char *macAddr);
        __declspec(dllexport) int pair_ganglion_native ();
        __declspec(dllexport) int stop_stream_native ();
        __declspec(dllexport) int start_stream_native ();
        __declspec(dllexport) int close_ganglion_native ();
        __declspec(dllexport) int get_data_native (struct GanglionDataNative *board_data);
    }
    #endif
}