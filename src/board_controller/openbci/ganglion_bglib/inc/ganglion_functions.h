#include "shared_export.h"

namespace GanglionLib
{
#ifdef __cplusplus
    extern "C"
    {
#endif
        // void * is a legacy from dynamic library which was loaded via LoadLibrary\dlopen
        SHARED_EXPORT int CALLING_CONVENTION initialize (void *param);
        SHARED_EXPORT int CALLING_CONVENTION open_ganglion (void *param);
        SHARED_EXPORT int CALLING_CONVENTION open_ganglion_mac_addr (void *param);
        SHARED_EXPORT int CALLING_CONVENTION stop_stream (void *param);
        SHARED_EXPORT int CALLING_CONVENTION start_stream (void *param);
        SHARED_EXPORT int CALLING_CONVENTION close_ganglion (void *param);
        SHARED_EXPORT int CALLING_CONVENTION get_data (void *param);
        SHARED_EXPORT int CALLING_CONVENTION config_board (void *param);
        SHARED_EXPORT int CALLING_CONVENTION release (void *param);
#ifdef __cplusplus
    }
#endif
} // namespace GanglionLib
