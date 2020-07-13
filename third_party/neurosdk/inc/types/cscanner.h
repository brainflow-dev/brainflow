#ifndef SCANNER_H
#define SCANNER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "clistener.h"
#include "lib_export.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __ANDROID__
#include <jni.h>
    typedef char AddressType[256];
#elif __APPLE__
typedef char AddressType[256];
#else
typedef uint64_t AddressType;
#endif

    typedef struct _DeviceEnumerator DeviceEnumerator;
    typedef struct _Device Device;

    typedef enum _DeviceType
    {
        DeviceTypeBrainbit,
        DeviceTypeCallibri,
        DeviceTypeAny
    } DeviceType;

    typedef struct _DeviceInfo
    {
        char Name[256];
        AddressType Address;
        uint64_t SerialNumber;
    } DeviceInfo;

    typedef struct _DeviceInfoArray
    {
        DeviceInfo *info_array;
        size_t info_count;
    } DeviceInfoArray;

    typedef void *DeviceListListener;

#ifdef __cplusplus
}
#endif

#endif // SCANNER_H
