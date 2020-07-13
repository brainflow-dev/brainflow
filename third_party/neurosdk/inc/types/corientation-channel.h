#ifndef CORIENTATION_CHANNEL_H
#define CORIENTATION_CHANNEL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "cchannels.h"

    typedef struct _OrientationChannel OrientationChannel;
    typedef struct _Quaternion
    {
        float W;
        float X;
        float Y;
        float Z;
    } Quaternion;

#ifdef __cplusplus
}
#endif

#endif // CRESISTANCE_CHANNEL_H
