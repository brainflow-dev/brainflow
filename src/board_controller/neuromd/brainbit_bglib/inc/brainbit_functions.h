#include "shared_export.h"

namespace BrainBitBLEDLib
{
#ifdef __cplusplus
    extern "C"
    {
#endif
        SHARED_EXPORT int CALLING_CONVENTION initialize (void *param);
        SHARED_EXPORT int CALLING_CONVENTION open_device (void *param);
        SHARED_EXPORT int CALLING_CONVENTION open_device_mac_addr (void *param);
        SHARED_EXPORT int CALLING_CONVENTION stop_stream (void *param);
        SHARED_EXPORT int CALLING_CONVENTION start_stream (void *param);
        SHARED_EXPORT int CALLING_CONVENTION close_device (void *param);
        SHARED_EXPORT int CALLING_CONVENTION get_data (void *param);
        SHARED_EXPORT int CALLING_CONVENTION release (void *param);
        SHARED_EXPORT int CALLING_CONVENTION config_device (void *param);
#ifdef __cplusplus
    }
#endif
} // namespace BrainBitBLEDLib
