#ifndef CFILTERED_CHANNEL_H
#define CFILTERED_CHANNEL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "cchannels.h"

typedef struct _FilteredDoubleChannel FilteredDoubleChannel;

SDK_SHARED FilteredDoubleChannel* create_FilteredDoubleChannel(DoubleChannel *channel, Filter *filters, size_t filter_count);
SDK_SHARED int FilteredDoubleChannel_get_buffer_size(FilteredDoubleChannel *channel, size_t *out_buffer_size);

#ifdef __cplusplus
}
#endif
	
#endif // CFILTERED_CHANNEL_H
