#pragma once

#include <simplecble/export.h>

#include <simplecble/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Returns the operating system on which SimpleBLE library is currently running.
 *
 * @return A simpleble_os_t value representing the current operating system.
 */
SIMPLECBLE_EXPORT simpleble_os_t simpleble_get_operating_system(void);

/**
 * Returns a string representing the version of the SimpleBLE library.
 *
 * @return A const char pointer to the version string.
 */
SIMPLECBLE_EXPORT const char* simpleble_get_version(void);

#ifdef __cplusplus
}
#endif
