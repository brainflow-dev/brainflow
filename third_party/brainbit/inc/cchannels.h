#ifndef CCHANNELS_H
#define CCHANNELS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "lib_export.h"
#include "cdevice.h"
#include "clistener.h"
#include <stdbool.h>
#include <stddef.h>


typedef struct _AnyChannel AnyChannel;
typedef struct _IntChannel IntChannel;
typedef struct _DoubleChannel DoubleChannel;
typedef struct _BridgeDoubleChannel BridgeDoubleChannel;

typedef enum _Filter {
	LowPass_1Hz_SF125,
	LowPass_1Hz_SF125_Reverse,
	LowPass_5Hz_SF125,
	LowPass_5Hz_SF125_Reverse,
	LowPass_15Hz_SF125,
	LowPass_15Hz_SF125_Reverse,
	LowPass_27Hz_SF125,
	LowPass_27Hz_SF125_Reverse,
	LowPass_30Hz_SF250,
	LowPass_30Hz_SF250_Reverse,

	HighPass_2Hz_SF250,
	HighPass_2Hz_SF250_Reverse,
	HighPass_3Hz_SF125,
	HighPass_3Hz_SF125_Reverse,
	HighPass_5Hz_SF125,
	HighPass_5Hz_SF125_Reverse,
	HighPass_11Hz_SF125,
	HighPass_11Hz_SF125_Reverse,

	BandStop_45_55Hz_SF250
} Filter;

SDK_SHARED void AnyChannel_delete(AnyChannel *channel);
SDK_SHARED int AnyChannel_get_info(AnyChannel *channel, ChannelInfo *out_info);
SDK_SHARED int AnyChannel_get_sampling_frequency(AnyChannel *channel, float * out_frequency);
SDK_SHARED int AnyChannel_add_length_callback(AnyChannel *channel, void(*callback)(AnyChannel *, size_t, void *), ListenerHandle *handle, void *user_data);
SDK_SHARED int AnyChannel_get_total_length(AnyChannel *channel, size_t *out_length);

SDK_SHARED int IntChannel_read_data(IntChannel *channel, size_t offset, size_t length, int *out_buffer, size_t buffer_size, size_t *samples_read);
SDK_SHARED int DoubleChannel_read_data(DoubleChannel *channel, size_t offset, size_t length, double *out_buffer, size_t buffer_size, size_t *samples_read);

#ifdef __cplusplus
}
#endif
	
#endif // CCHANNELS_H
