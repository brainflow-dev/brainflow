#ifndef SCANNER_H
#define SCANNER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "lib_export.h"

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

typedef enum _DeviceType {
	DeviceTypeBrainbit,
	DeviceTypeCallibri,
	DeviceTypeAny
} DeviceType;

typedef struct _DeviceInfo {
	char Name[256];
	AddressType Address;
	uint64_t SerialNumber;
} DeviceInfo;

typedef struct _DeviceInfoArray {
	DeviceInfo *info_array;
	size_t info_count;
} DeviceInfoArray;

#ifdef __ANDROID__
SDK_SHARED DeviceEnumerator* create_device_enumerator(JNIEnv *env, jobject context, DeviceType);
#else
SDK_SHARED DeviceEnumerator* create_device_enumerator(DeviceType);
#endif

typedef void* DeviceListListener;

SDK_SHARED void enumerator_delete(DeviceEnumerator *);
SDK_SHARED int enumerator_set_device_list_changed_callback(DeviceEnumerator *, void(*)(DeviceEnumerator *, void *), DeviceListListener *, void *user_data);
SDK_SHARED void enumerator_unsubscribe_device_list_changed(DeviceListListener);
SDK_SHARED int enumerator_get_device_list(DeviceEnumerator *, DeviceInfoArray *out_device_array);

SDK_SHARED void free_DeviceInfoArray(DeviceInfoArray);

#ifndef __ANDROID__
	#ifndef __APPLE__
		SDK_SHARED AddressType address_from_string(char* address);
		SDK_SHARED int address_to_string(AddressType address, char* out_string, size_t out_string_size);
	#endif
#endif

#ifdef __cplusplus
}
#endif
	
#endif // SCANNER_H
