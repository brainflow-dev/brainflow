#pragma once

#include <string.h>

#pragma pack(push, 1)
struct GForceData
{
    double data[8];
    double timestamp;

    GForceData (double *data, double timestamp)
    {
        memcpy (this->data, data, sizeof (double) * 8);
        this->timestamp = timestamp;
    }

    GForceData ()
    {
        timestamp = 0.0;
    }

    GForceData (const GanglionData &other)
    {
        timestamp = other.timestamp;
        memcpy (data, other.data, sizeof (double) * 8);
    }
};
#pragma pack(pop)

enum class CustomExitCodes : int
{
    STATUS_OK = 0,
    HUB_INIT_FAILED = 1,
    SCAN_INIT_FAILED = 2,
    NO_DEVICE_FOUND = 3,
    CONNECT_ERROR = 4,
    ALREADY_INITIALIZED = 5,
    NOT_INITIALIZED = 6,
    SERIAL_DETECT_ERROR = 7,
    FOUND_BUT_IN_CONNECTING_STATE = 8,
    SYNC_ERROR = 9
};