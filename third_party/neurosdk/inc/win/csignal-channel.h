#ifndef CSIGNAL_CHANNEL_H
#define CSIGNAL_CHANNEL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "cchannels.h"

typedef struct _SignalDoubleChannel SignalDoubleChannel;

typedef struct _SignalDataArray {
	double *data_array;
	size_t samples_count;
	uint16_t first_samples_number;
} SignalDataArray;

SDK_SHARED int device_subscribe_signal_channel_data_received(Device*, ChannelInfo, void(*)(Device*, ChannelInfo, SignalDataArray, void *), ListenerHandle *, void *user_data);
SDK_SHARED SignalDoubleChannel* create_SignalDoubleChannel(Device *device_ptr);
SDK_SHARED SignalDoubleChannel* create_SignalDoubleChannel_info(Device *device_ptr, ChannelInfo info);
SDK_SHARED SignalDoubleChannel* create_SignalDoubleChannel_info_filters(Device *device_ptr, ChannelInfo info, Filter *filters, size_t filter_count);
SDK_SHARED SignalDoubleChannel* create_SignalDoubleChannel_filters(Device *device_ptr, Filter *filters, size_t filter_count);
SDK_SHARED int SignalDoubleChannel_get_buffer_size(SignalDoubleChannel *channel, size_t *out_buffer_size);

SDK_SHARED void free_SignalDataArray(SignalDataArray data_array);

#ifdef __cplusplus
}
#endif
	
#endif // CSIGNAL_CHANNEL_H
