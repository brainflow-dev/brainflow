#pragma once

#include <thread>

#include "board.h"
#include "board_controller.h"

#if defined _WIN32 || defined __APPLE__
#include "cdevice.h"
#include "clistener.h"
#include "csignal-channel.h"
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

#endif

public:
    NeuromdBoard (int board_id, struct BrainFlowInputParams params);
    virtual ~NeuromdBoard ();
};
