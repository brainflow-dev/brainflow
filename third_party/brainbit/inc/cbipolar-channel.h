#ifndef CBIPOLAR_CHANNEL_H
#define CBIPOLAR_CHANNEL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "cchannels.h"

typedef struct _BipolarDoubleChannel BipolarDoubleChannel;

SDK_SHARED BipolarDoubleChannel* create_BipolarDoubleChannel(DoubleChannel *first, DoubleChannel *second);

#ifdef __cplusplus
}
#endif
	
#endif // CBIPOLAR_CHANNEL_H
