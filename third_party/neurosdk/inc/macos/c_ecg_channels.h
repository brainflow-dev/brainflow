#ifndef ECG_EXTENSIONS_CHANNELS_H
#define ECG_EXTENSIONS_CHANNELS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "lib_export.h"
#include "csignal-channel.h"
#include "celectrode-state-channel.h"

typedef struct _EcgDoubleChannel EcgDoubleChannel;

SDK_SHARED EcgDoubleChannel* create_EcgDoubleChannel(Device *device_ptr);
SDK_SHARED EcgDoubleChannel* create_EcgDoubleChannel_info(Device *device_ptr, ChannelInfo info);
SDK_SHARED int EcgDoubleChannel_get_buffer_size(EcgDoubleChannel *channel, size_t *out_buffer_size);


typedef struct _RPeakChannel RPeakChannel;

SDK_SHARED RPeakChannel* create_RPeakChannel(SignalDoubleChannel *);
SDK_SHARED RPeakChannel* create_RPeakChannel_ElectrodeState(SignalDoubleChannel *, ElectrodeStateChannel *);
SDK_SHARED int RPeakChannel_read_data(RPeakChannel *channel, size_t offset, size_t length, size_t *out_buffer);

typedef struct _HeartRateIntChannel HeartRateIntChannel;

SDK_SHARED HeartRateIntChannel* create_HeartRateIntChannel(RPeakChannel *);


typedef struct _StressIndexDoubleChannel StressIndexDoubleChannel;

SDK_SHARED StressIndexDoubleChannel* create_StressIndexDoubleChannel(RPeakChannel *);

#ifdef __cplusplus
}
#endif
	
#endif
