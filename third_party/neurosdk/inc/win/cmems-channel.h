#ifndef CMEMS_CHANNEL_H
#define CMEMS_CHANNEL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "cchannels.h"

typedef struct _MEMSChannel MEMSChannel;
typedef struct _MEMS {
	struct Accelerometer {
		double X;
		double Y;
		double Z;
	} accelerometer;

	struct Gyroscope {
		double X;
		double Y;
		double Z;
	} gyroscope;
} MEMS;

SDK_SHARED MEMSChannel* create_MEMSChannel(Device *device_ptr);
SDK_SHARED int MEMSChannel_get_buffer_size(MEMSChannel *channel, size_t *out_buffer_size);
SDK_SHARED int MEMSChannel_read_data(MEMSChannel *channel, size_t offset, size_t length, MEMS *out_buffer, size_t buffer_size, size_t *samples_read);

#ifdef __cplusplus
}
#endif
	
#endif // CRESISTANCE_CHANNEL_H
