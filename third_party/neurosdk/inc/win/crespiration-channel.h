#ifndef CRESPIRATION_CHANNEL_H
#define CRESPIRATION_CHANNEL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "cchannels.h"

typedef struct _RespirationDoubleChannel RespirationDoubleChannel;

SDK_SHARED RespirationDoubleChannel* create_RespirationDoubleChannel(Device *device_ptr);
SDK_SHARED int RespirationDoubleChannel_get_buffer_size(RespirationDoubleChannel *channel, size_t *out_buffer_size);

#ifdef __cplusplus
}
#endif
	
#endif // CRESISTANCE_CHANNEL_H
