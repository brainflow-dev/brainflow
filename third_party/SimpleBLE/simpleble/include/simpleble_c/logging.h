#pragma once

#include <stdint.h>

#include <simpleble/export.h>

#ifdef _WIN32
#define SHARED_EXPORT __declspec(dllexport)
#define CALLING_CONVENTION __cdecl
#else
#define SHARED_EXPORT __attribute__((visibility("default")))
#define CALLING_CONVENTION
#endif

typedef enum {
    SIMPLEBLE_LOG_LEVEL_NONE = 0,
    SIMPLEBLE_LOG_LEVEL_FATAL,
    SIMPLEBLE_LOG_LEVEL_ERROR,
    SIMPLEBLE_LOG_LEVEL_WARN,
    SIMPLEBLE_LOG_LEVEL_INFO,
    SIMPLEBLE_LOG_LEVEL_DEBUG,
    SIMPLEBLE_LOG_LEVEL_VERBOSE
} simpleble_log_level_t;

// clang-format off
typedef void (*simpleble_log_callback_t)(
    simpleble_log_level_t level,
    const char* module,
    const char* file,
    uint32_t line,
    const char* function,
    const char* message
);
// clang-format on

#ifdef __cplusplus
extern "C" {
#endif

SHARED_EXPORT void CALLING_CONVENTION simpleble_logging_set_level(simpleble_log_level_t level);
SHARED_EXPORT void CALLING_CONVENTION simpleble_logging_set_callback(simpleble_log_callback_t callback);

#ifdef __cplusplus
}
#endif
