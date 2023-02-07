#pragma once

#include <simpleble/export.h>
#include <simpleble_c/adapter.h>
#include <simpleble_c/peripheral.h>
#include <simpleble_c/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Wrapper around free() to allow memory to be cleared
 *        within the library.
 *
 * @param handle
 */
SHARED_EXPORT void CALLING_CONVENTION simpleble_free(void* handle);

#ifdef __cplusplus
}
#endif
