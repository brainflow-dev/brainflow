#ifndef CBATTERY_CHANNEL_H
#define CBATTERY_CHANNEL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "cchannels.h"

	typedef struct _BatteryIntChannel BatteryIntChannel;

	SDK_SHARED BatteryIntChannel* create_BatteryIntChannel(Device* device_ptr);
	SDK_SHARED int BatteryIntChannel_get_buffer_size(BatteryIntChannel* channel, size_t* out_buffer_size);

#ifdef __cplusplus
}
#endif
	
#endif // CBATTERY_CHANNEL_H
