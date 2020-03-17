#ifndef CORIENTATION_CHANNEL_H
#define CORIENTATION_CHANNEL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "cchannels.h"

typedef struct _OrientationChannel OrientationChannel;
typedef struct _Quaternion {
	float W;
	float X;
	float Y;
	float Z;
} Quaternion;


SDK_SHARED OrientationChannel* create_OrientationChannel(Device *device_ptr);
SDK_SHARED int OrientationChannel_get_buffer_size(OrientationChannel *channel, size_t *out_buffer_size);
SDK_SHARED int OrientationChannel_read_data(OrientationChannel *channel, size_t offset, size_t length, Quaternion *out_buffer, size_t buffer_size, size_t *samples_read);

#ifdef __cplusplus
}
#endif
	
#endif // CRESISTANCE_CHANNEL_H
