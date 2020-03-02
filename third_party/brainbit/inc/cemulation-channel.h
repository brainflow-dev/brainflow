#ifndef CEMULATION_CHANNEL_H
#define CEMULATION_CHANNEL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "cchannels.h"

typedef struct _EmulationDoubleChannel EmulationDoubleChannel;

typedef struct _EmulationSine {
	double AmplitudeV;
	double FrequencyHz;
	double PhaseShiftRad;
} EmulationSine;

SDK_SHARED EmulationDoubleChannel* create_EmulationDoubleChannel(EmulationSine *components, size_t components_count, float sampling_frequency, size_t initial_length);
SDK_SHARED int EmulationDoubleChannel_start_timer(EmulationDoubleChannel* channel);
SDK_SHARED int EmulationDoubleChannel_stop_timer(EmulationDoubleChannel* channel);
SDK_SHARED int EmulationDoubleChannel_reset(EmulationDoubleChannel* channel);

#ifdef __cplusplus
}
#endif
	
#endif // CEMULATION_CHANNEL_H
