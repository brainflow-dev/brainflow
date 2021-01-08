#ifndef SDK_ERROR_H
#define SDK_ERROR_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "lib_export.h"
#include <stddef.h>

    typedef int ret_code;

#ifndef SDK_NO_ERROR
#define SDK_NO_ERROR 0
#endif
#ifndef ERROR_UNHANDLED_EXCEPTION
#define ERROR_UNHANDLED_EXCEPTION 1
#endif
#ifndef ERROR_EXCEPTION_WITH_MESSAGE
#define ERROR_EXCEPTION_WITH_MESSAGE 2
#endif

#ifdef __cplusplus
}
#endif

#endif // SDK_ERROR_H
