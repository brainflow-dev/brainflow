#ifndef CMEMS_CHANNEL_H
#define CMEMS_CHANNEL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "cchannels.h"

    typedef struct _MEMSChannel MEMSChannel;
    typedef struct _MEMS
    {
        struct Accelerometer
        {
            double X;
            double Y;
            double Z;
        } accelerometer;

        struct Gyroscope
        {
            double X;
            double Y;
            double Z;
        } gyroscope;
    } MEMS;

#ifdef __cplusplus
}
#endif

#endif // CRESISTANCE_CHANNEL_H
