#ifndef CELECTRODE_STATE_CHANNEL_H
#define CELECTRODE_STATE_CHANNEL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "cchannels.h"

    typedef struct _ElectrodeStateChannel ElectrodeStateChannel;

    typedef enum _ElectrodeState
    {
        ElectrodeStateNormal,
        ElectrodeStateHighResistance,
        ElectrodeStateDetached
    } ElectrodeState;

#ifdef __cplusplus
}
#endif

#endif // CRESISTANCE_CHANNEL_H
