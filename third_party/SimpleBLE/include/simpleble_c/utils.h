#pragma once

#include <simpleble_c/shared_export.h>
#include <simpleble_c/types.h>

#ifdef __cplusplus
extern "C" {
#endif

SHARED_EXPORT simpleble_os_t CALLING_CONVENTION get_operating_system(void);

#ifdef __cplusplus
}
#endif
