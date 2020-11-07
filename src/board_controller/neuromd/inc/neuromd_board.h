#pragma once

#include <thread>

#include "board.h"
#include "board_controller.h"
#include "runtime_dll_loader.h"

#if defined _WIN32 || defined __APPLE__
#include "cdevice.h"
#include "clistener.h"
#include "cparams.h"
#include "cscanner.h"
#include "csignal-channel.h"
#include "sdk_error.h"
#endif

class NeuromdBoard : public Board
{

protected:
    // as for now NeuroSDK supports only windows and macos, to dont write ifdef in
    // board_controller.cpp add dummy implementation for linux
#if defined _WIN32 || defined __APPLE__

    Device *device;

    int find_device ();
    int find_device_info (DeviceEnumerator *enumerator, DeviceInfo *out_device_info);
    int connect_device ();
    void free_device ();
    void free_listener (ListenerHandle lh);

    DLLLoader *dll_loader;
    bool initialized;
    // init all required function pointers in single place
    int (*sdk_last_error_msg) (char *, size_t);
    int (*device_disconnect) (Device *);
    void (*device_delete) (Device *);
    DeviceEnumerator *(*create_device_enumerator) (DeviceType);
    Device *(*create_Device) (DeviceEnumerator *, DeviceInfo);
    void (*enumerator_delete) (DeviceEnumerator *);
    int (*enumerator_get_device_list) (DeviceEnumerator *, DeviceInfoArray *);
    void (*free_DeviceInfoArray) (DeviceInfoArray);
    int (*device_read_Name) (Device *, char *, size_t);
    int (*device_read_SerialNumber) (Device *, char *, size_t);
    int (*device_connect) (Device *);
    int (*device_read_State) (Device *, DeviceState *);
    int (*device_set_SamplingFrequency) (Device *, SamplingFrequency);
    int (*device_set_Gain) (Device *, Gain);
    int (*device_set_Offset) (Device *, unsigned char);
    int (*device_set_ExternalSwitchState) (Device *, ExternalSwitchInput);
    int (*device_set_ADCInputState) (Device *, ADCInput);
    int (*device_set_HardwareFilterState) (Device *, bool);
    int (*device_available_channels) (const Device *, ChannelInfoArray *);
    SignalDoubleChannel *(*create_SignalDoubleChannel_info) (Device *, ChannelInfo);
    void (*free_ChannelInfoArray) (ChannelInfoArray);
    int (*device_execute) (Device *, Command);
    int (*AnyChannel_get_total_length) (AnyChannel *, size_t *);
    int (*DoubleChannel_read_data) (DoubleChannel *, size_t, size_t, double *, size_t, size_t *);
    void (*AnyChannel_delete) (AnyChannel *);
    int (*device_subscribe_int_channel_data_received) (Device *, ChannelInfo,
        void (*) (Device *, ChannelInfo, IntDataArray, void *), ListenerHandle *, void *);
    int (*device_subscribe_double_channel_data_received) (Device *, ChannelInfo,
        void (*) (Device *, ChannelInfo, DoubleDataArray, void *), ListenerHandle *, void *);
    void (*free_IntDataArray) (IntDataArray);
    void (*free_DoubleDataArray) (DoubleDataArray);

// different headers for windows and apple
#ifdef _WIN32
    void (*free_listener_handle) (ListenerHandle handle);
#else
    void (*free_length_listener_handle) (LengthListenerHandle handle);
#endif

#endif

public:
    virtual int prepare_session ();
    virtual int release_session ();

    NeuromdBoard (int board_id, struct BrainFlowInputParams params);
    virtual ~NeuromdBoard ();
};
