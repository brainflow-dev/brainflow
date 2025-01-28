#include "shared_export.h"


#ifdef __cplusplus
extern "C"
{
#endif
    SHARED_EXPORT int CALLING_CONVENTION synchroni_initialize (void *param);
    SHARED_EXPORT int CALLING_CONVENTION synchroni_open_device (void *param);
    SHARED_EXPORT int CALLING_CONVENTION synchroni_stop_stream (void *param);
    SHARED_EXPORT int CALLING_CONVENTION synchroni_start_stream (void *param);
    SHARED_EXPORT int CALLING_CONVENTION synchroni_close_device (void *param);
    SHARED_EXPORT int CALLING_CONVENTION synchroni_get_data_default (void *param);
    SHARED_EXPORT int CALLING_CONVENTION synchroni_get_data_aux (void *param);
    SHARED_EXPORT int CALLING_CONVENTION synchroni_get_data_anc (void *param);
    SHARED_EXPORT int CALLING_CONVENTION synchroni_release (void *param);
    SHARED_EXPORT int CALLING_CONVENTION synchroni_config_device (void *param);
#ifdef __cplusplus
}
#endif
