#ifndef CRESISTANCE_CHANNEL_H
#define CRESISTANCE_CHANNEL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "cchannels.h"

typedef struct _ResistanceDoubleChannel ResistanceDoubleChannel;

SDK_SHARED ResistanceDoubleChannel* create_ResistanceDoubleChannel_info(Device *device_ptr, ChannelInfo info);
SDK_SHARED int ResistanceDoubleChannel_get_buffer_size(ResistanceDoubleChannel *channel, size_t *out_buffer_size);

#ifdef __cplusplus
}
#endif
	
#endif // CRESISTANCE_CHANNEL_H
