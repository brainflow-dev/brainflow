#ifndef CBRIDGE_CHANNELS_H
#define CBRIDGE_CHANNELS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "cchannels.h"

typedef struct _BridgeDoubleChannel BridgeDoubleChannel;

typedef int(*ReadDataFunc)(size_t offset, size_t length, double *out_buffer);
typedef int(*GetFrequencyFunc)(float * out_frequency);
typedef int(*AddLengthCallbackFunc)(void(*callback)(BridgeDoubleChannel *, size_t), ListenerHandle *handle);
typedef int(*GetTotalLengthFunc)(size_t *out_length);

SDK_SHARED BridgeDoubleChannel* create_BridgeDoubleChannel_info(ChannelInfo, ReadDataFunc, GetFrequencyFunc, AddLengthCallbackFunc, GetTotalLengthFunc);

#ifdef __cplusplus
}
#endif
	
#endif // CBRIDGE_CHANNELS_H
