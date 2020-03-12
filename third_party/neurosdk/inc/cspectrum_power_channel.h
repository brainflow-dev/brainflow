#ifndef C_SPECTRUM_POWER_CHANNEL_H
#define C_SPECTRUM_POWER_CHANNEL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "lib_export.h"
#include "cspectrum-channel.h"

typedef struct _SpectrumPowerDoubleChannel SpectrumPowerDoubleChannel;

SDK_SHARED SpectrumPowerDoubleChannel* create_SpectrumPowerDoubleChannel(
	SpectrumDoubleChannel **channels, 
	size_t channels_count, 
	float low_freq, float high_freq,
	const char *name,
	double window_duration, double overlapping_coeff);

SDK_SHARED int SpectrumPowerDoubleChannel_set_window_duration(SpectrumPowerDoubleChannel *channel, double duration);
SDK_SHARED int SpectrumPowerDoubleChannel_set_frequency_band(SpectrumPowerDoubleChannel *channel, float low_freq, float high_freq);
SDK_SHARED int SpectrumPowerDoubleChannel_set_overlapping_coefficient(SpectrumPowerDoubleChannel *channel, double overlap);

SDK_SHARED double spectrum_power_normalized(float low_frequency, float high_frequency, double *spectrum, size_t spectrum_length, float frequency_step);

#ifdef __cplusplus
}
#endif
	
#endif
