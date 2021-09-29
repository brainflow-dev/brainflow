#pragma once

#include <simpleble_c/adapter.h>
#include <simpleble_c/peripheral.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Wrapper around free() to allow memory to be cleared
 *        within the library.
 *
 * @param handle
 */
void simpleble_free(void* handle);

#ifdef __cplusplus
}
#endif
