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
    SHARED_EXPORT int CALLING_CONVENTION gforceInitialize (void *param);
    SHARED_EXPORT int CALLING_CONVENTION gforceStartStreaming (void *param);
    SHARED_EXPORT int CALLING_CONVENTION gforceStopStreaming (void *param);
    SHARED_EXPORT int CALLING_CONVENTION gforceGetData (void *param);
    SHARED_EXPORT int CALLING_CONVENTION gforceRelease (void *param);
#ifdef __cplusplus
}
#endif
