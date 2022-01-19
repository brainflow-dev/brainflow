#pragma once

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

    SHARED_EXPORT int CALLING_CONVENTION prepare ();
    SHARED_EXPORT int CALLING_CONVENTION predict (double *, int, double *);
    SHARED_EXPORT int CALLING_CONVENTION release ();

#ifdef __cplusplus
}
#endif
