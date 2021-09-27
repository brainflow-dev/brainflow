#pragma once

#include <simpleble_c/adapter.h>
#include <simpleble_c/peripheral.h>

#ifdef _WIN32
#define EXPORT_SYMBOL __declspec(dllexport)
#else
#define EXPORT_SYMBOL
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Wrapper around free() to allow memory to be cleared
 *        within the library.
 *
 * @param handle
 */
EXPORT_SYMBOL void simpleble_free(void* handle);

#ifdef __cplusplus
}
#endif
