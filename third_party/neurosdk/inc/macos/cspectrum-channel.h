#ifndef CSPECTRUM_CHANNEL_H
#define CSPECTRUM_CHANNEL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "cchannels.h"

typedef struct _SpectrumDoubleChannel SpectrumDoubleChannel;
typedef enum _SpectrumWindow {
	SpectrumWindowRectangular,
	SpectrumWindowSine,
	SpectrumWindowHamming,
	SpectrumWindowBlackman
} SpectrumWindow;

SDK_SHARED SpectrumDoubleChannel* create_SpectrumDoubleChannel(DoubleChannel *);
SDK_SHARED int SpectrumDoubleChannel_read_data(SpectrumDoubleChannel *channel, size_t offset, size_t length, double *out_buffer, size_t buffer_size, size_t *samples_read, SpectrumWindow window_type);
SDK_SHARED int SpectrumDoubleChannel_get_hz_per_spectrum_sample(SpectrumDoubleChannel* channel, double* out_step);
SDK_SHARED int SpectrumDoubleChannel_get_spectrum_length(SpectrumDoubleChannel *channel, size_t *out_spectrum_length);

#ifdef __cplusplus
}
#endif
	
#endif // CSPECTRUM_CHANNEL_H
