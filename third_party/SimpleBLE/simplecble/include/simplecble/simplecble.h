#pragma once

#include <simplecble/export.h>
#include <simplecble/adapter.h>
#include <simplecble/peripheral.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Wrapper around free() to allow memory to be cleared
 *        within the library.
 *
 * @param handle
 */
SIMPLECBLE_EXPORT void simpleble_free(void* handle);

#ifdef __cplusplus
}
#endif
