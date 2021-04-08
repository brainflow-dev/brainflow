#ifdef _WIN32
#define SHARED_EXPORT __declspec(dllexport)
#define CALLING_CONVENTION __cdecl
#else
#define SHARED_EXPORT __attribute__ ((visibility ("default")))
#define CALLING_CONVENTION
#endif


#ifdef __cplusplus
extern "C"
{
#endif
    SHARED_EXPORT int CALLING_CONVENTION initialize (void *param);
    SHARED_EXPORT int CALLING_CONVENTION open_device (void *param);
    SHARED_EXPORT int CALLING_CONVENTION stop_stream (void *param);
    SHARED_EXPORT int CALLING_CONVENTION start_stream (void *param);
    SHARED_EXPORT int CALLING_CONVENTION close_device (void *param);
    SHARED_EXPORT int CALLING_CONVENTION get_data (void *param);
    SHARED_EXPORT int CALLING_CONVENTION release (void *param);
    SHARED_EXPORT int CALLING_CONVENTION config_device (void *param);
#ifdef __cplusplus
}
#endif
