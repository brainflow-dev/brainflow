#ifndef CLISTENER_H
#define CLISTENER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "lib_export.h"

typedef void* ListenerHandle;
SDK_SHARED void free_listener_handle(ListenerHandle handle);

#ifdef __cplusplus
}
#endif
	
#endif // CLISTENER_H
