#ifndef CSIGNAL_CHANNEL_H
#define CSIGNAL_CHANNEL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "cchannels.h"

    typedef struct _SignalDoubleChannel SignalDoubleChannel;

    typedef struct _SignalDataArray
    {
        double *data_array;
        size_t samples_count;
        uint16_t first_samples_number;
    } SignalDataArray;

#ifdef __cplusplus
}
#endif

#endif // CSIGNAL_CHANNEL_H
