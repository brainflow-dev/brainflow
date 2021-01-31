#pragma once

#include <string.h>

#pragma pack(push, 1)
struct GforceData
{
    static const int SIZE = 11;

    double data[SIZE];

    GforceData (double *data)
    {
        memcpy (this->data, data, sizeof (double) * SIZE);
    }

    GforceData ()
    {
        for (int i = 0; i < SIZE; i++)
        {
            data[i] = 0;
        }
    }

    GforceData (const GforceData &other)
    {
        memcpy (data, other.data, sizeof (double) * SIZE);
    }
};
#pragma pack(pop)

enum class GforceWrapperExitCodes : int
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
    SYNC_ERROR = 9,
    RUN_ERROR = 10,
    NO_DATA_ERROR = 11
};