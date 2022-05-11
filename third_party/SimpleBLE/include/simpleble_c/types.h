#pragma once

#include <stddef.h>

#define SIMPLEBLE_UUID_STR_LEN 37  // 36 characters + null terminator
#define SIMPLEBLE_CHARACTERISTIC_MAX_COUNT 16

// TODO: Add proper error codes.
typedef enum {
    SIMPLEBLE_SUCCESS = 0,
    SIMPLEBLE_FAILURE = 1,
} simpleble_err_t;

typedef struct {
    char value[SIMPLEBLE_UUID_STR_LEN];
} simpleble_uuid_t;

typedef struct {
    simpleble_uuid_t uuid;
    size_t characteristic_count;
    simpleble_uuid_t characteristics[SIMPLEBLE_CHARACTERISTIC_MAX_COUNT];
} simpleble_service_t;

typedef struct {
    uint16_t manufacturer_id;
    size_t data_length;
    uint8_t data[24];
} simpleble_manufacturer_data_t;

typedef void* simpleble_adapter_t;
typedef void* simpleble_peripheral_t;
typedef void* simpleble_characteristic_t;
