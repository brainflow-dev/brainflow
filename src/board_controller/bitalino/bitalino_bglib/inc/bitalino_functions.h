#include "shared_export.h"

namespace BitalinoLib
{
#ifdef __cplusplus
    extern "C"
    {
#endif
        SHARED_EXPORT int CALLING_CONVENTION initialize (void *param);
        SHARED_EXPORT int CALLING_CONVENTION open_bitalino (void *param);
        SHARED_EXPORT int CALLING_CONVENTION open_bitalino_mac_addr (void *param);
        SHARED_EXPORT int CALLING_CONVENTION stop_stream (void *param);
        SHARED_EXPORT int CALLING_CONVENTION config_board (void *param);
        SHARED_EXPORT int CALLING_CONVENTION start_stream (void *param);
        SHARED_EXPORT int CALLING_CONVENTION close_bitalino (void *param);
        SHARED_EXPORT int CALLING_CONVENTION get_data (void *param);
        SHARED_EXPORT int CALLING_CONVENTION release (void *param);
#ifdef __cplusplus
    }
#endif
} // namespace BitalinoLib
