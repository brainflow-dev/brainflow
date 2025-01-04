#pragma once

#ifdef _WIN32
#define SHARED_EXPORT __declspec(dllexport)
#define CALLING_CONVENTION __cdecl
#else
#define SHARED_EXPORT __attribute__((visibility("default")))
#define CALLING_CONVENTION
#endif

#include <simpleble_c/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Returns the operating system on which SimpleBLE library is currently running.
 *
 * @return A simpleble_os_t value representing the current operating system.
 */
SHARED_EXPORT simpleble_os_t simpleble_get_operating_system(void);

/**
 * Returns a string representing the version of the SimpleBLE library.
 *
 * @return A const char pointer to the version string.
 */
SHARED_EXPORT const char* CALLING_CONVENTION simpleble_get_version(void);

#ifdef __cplusplus
}
#endif
