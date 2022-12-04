#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <simpleble/export.h>

#include <simpleble_c/types.h>

#ifdef _WIN32
#define SHARED_EXPORT __declspec(dllexport)
#define CALLING_CONVENTION __cdecl
#else
#define SHARED_EXPORT __attribute__((visibility("default")))
#define CALLING_CONVENTION
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Releases all memory and resources consumed by the specific
 *        instance of simpleble_peripheral_t.
 *
 * @param handle
 */
SHARED_EXPORT void CALLING_CONVENTION simpleble_peripheral_release_handle(simpleble_peripheral_t handle);

/**
 * @brief
 *
 * @param handle
 * @return char*
 */
SHARED_EXPORT char* CALLING_CONVENTION simpleble_peripheral_identifier(simpleble_peripheral_t handle);

/**
 * @brief
 *
 * @param handle
 * @return char*
 */
SHARED_EXPORT char* CALLING_CONVENTION simpleble_peripheral_address(simpleble_peripheral_t handle);

/**
 * @brief
 *
 * @param handle
 * @return int16_t
 */
SHARED_EXPORT int16_t CALLING_CONVENTION simpleble_peripheral_rssi(simpleble_peripheral_t handle);

/**
 * @brief
 *
 * @param handle
 * @return uint16_t
 */
SHARED_EXPORT uint16_t CALLING_CONVENTION simpleble_peripheral_mtu(simpleble_peripheral_t handle);

/**
 * @brief
 *
 * @param handle
 * @return simpleble_err_t
 */
SHARED_EXPORT simpleble_err_t CALLING_CONVENTION simpleble_peripheral_connect(simpleble_peripheral_t handle);

/**
 * @brief
 *
 * @param handle
 * @return simpleble_err_t
 */
SHARED_EXPORT simpleble_err_t CALLING_CONVENTION simpleble_peripheral_disconnect(simpleble_peripheral_t handle);

/**
 * @brief
 *
 * @param handle
 * @param connected
 * @return simpleble_err_t
 */
SHARED_EXPORT simpleble_err_t CALLING_CONVENTION simpleble_peripheral_is_connected(simpleble_peripheral_t handle,
                                                                                   bool* connected);

/**
 * @brief
 *
 * @param handle
 * @param connectable
 * @return simpleble_err_t
 */
SHARED_EXPORT simpleble_err_t CALLING_CONVENTION simpleble_peripheral_is_connectable(simpleble_peripheral_t handle,
                                                                                     bool* connectable);

/**
 * @brief
 *
 * @param handle
 * @param paired
 * @return simpleble_err_t
 */
SHARED_EXPORT simpleble_err_t CALLING_CONVENTION simpleble_peripheral_is_paired(simpleble_peripheral_t handle,
                                                                                bool* paired);

/**
 * @brief
 *
 * @param handle
 * @return simpleble_err_t
 */
SHARED_EXPORT simpleble_err_t CALLING_CONVENTION simpleble_peripheral_unpair(simpleble_peripheral_t handle);

/**
 * @brief
 *
 * @param handle
 * @return size_t
 */
SHARED_EXPORT size_t CALLING_CONVENTION simpleble_peripheral_services_count(simpleble_peripheral_t handle);

/**
 * @brief
 *
 * @param handle
 * @param index
 * @param services
 * @return simpleble_err_t
 */
SHARED_EXPORT simpleble_err_t CALLING_CONVENTION simpleble_peripheral_services_get(simpleble_peripheral_t handle,
                                                                                   size_t index,
                                                                                   simpleble_service_t* services);

/**
 * @brief
 *
 * @param handle
 * @return size_t
 */
SHARED_EXPORT size_t CALLING_CONVENTION simpleble_peripheral_manufacturer_data_count(simpleble_peripheral_t handle);

/**
 * @brief
 *
 * @param handle
 * @param index
 * @param manufacturer_data
 * @return simpleble_err_t
 */
SHARED_EXPORT simpleble_err_t CALLING_CONVENTION simpleble_peripheral_manufacturer_data_get(
    simpleble_peripheral_t handle, size_t index, simpleble_manufacturer_data_t* manufacturer_data);

/**
 * @brief
 *
 * @note The user is responsible for freeing the pointer returned in data.
 *
 * @param handle
 * @param service
 * @param characteristic
 * @param data
 * @param data_length
 * @return simpleble_err_t
 */
SHARED_EXPORT simpleble_err_t CALLING_CONVENTION simpleble_peripheral_read(simpleble_peripheral_t handle,
                                                                           simpleble_uuid_t service,
                                                                           simpleble_uuid_t characteristic,
                                                                           uint8_t** data, size_t* data_length);

/**
 * @brief
 *
 * @param handle
 * @param service
 * @param characteristic
 * @param data
 * @param data_length
 * @return simpleble_err_t
 */
SHARED_EXPORT simpleble_err_t CALLING_CONVENTION simpleble_peripheral_write_request(simpleble_peripheral_t handle,
                                                                                    simpleble_uuid_t service,
                                                                                    simpleble_uuid_t characteristic,
                                                                                    const uint8_t* data,
                                                                                    size_t data_length);

/**
 * @brief
 *
 * @param handle
 * @param service
 * @param characteristic
 * @param data
 * @param data_length
 * @return simpleble_err_t
 */
SHARED_EXPORT simpleble_err_t CALLING_CONVENTION simpleble_peripheral_write_command(simpleble_peripheral_t handle,
                                                                                    simpleble_uuid_t service,
                                                                                    simpleble_uuid_t characteristic,
                                                                                    const uint8_t* data,
                                                                                    size_t data_length);

/**
 * @brief
 *
 * @param handle
 * @param service
 * @param characteristic
 * @param callback
 * @return simpleble_err_t
 */
SHARED_EXPORT simpleble_err_t CALLING_CONVENTION
simpleble_peripheral_notify(simpleble_peripheral_t handle, simpleble_uuid_t service, simpleble_uuid_t characteristic,
                            void (*callback)(simpleble_uuid_t service, simpleble_uuid_t characteristic,
                                             const uint8_t* data, size_t data_length, void* userdata),
                            void* userdata);

/**
 * @brief
 *
 * @param handle
 * @param service
 * @param characteristic
 * @param callback
 * @return simpleble_err_t
 */
SHARED_EXPORT simpleble_err_t CALLING_CONVENTION
simpleble_peripheral_indicate(simpleble_peripheral_t handle, simpleble_uuid_t service, simpleble_uuid_t characteristic,
                              void (*callback)(simpleble_uuid_t service, simpleble_uuid_t characteristic,
                                               const uint8_t* data, size_t data_length, void* userdata),
                              void* userdata);

/**
 * @brief
 *
 * @param handle
 * @param service
 * @param characteristic
 * @return simpleble_err_t
 */
SHARED_EXPORT simpleble_err_t CALLING_CONVENTION simpleble_peripheral_unsubscribe(simpleble_peripheral_t handle,
                                                                                  simpleble_uuid_t service,
                                                                                  simpleble_uuid_t characteristic);

/**
 * @brief
 *
 * @param handle
 * @param service
 * @param characteristic
 * @param descriptor
 * @param data
 * @param data_length
 * @return simpleble_err_t
 */
SHARED_EXPORT simpleble_err_t CALLING_CONVENTION simpleble_peripheral_read_descriptor(
    simpleble_peripheral_t handle, simpleble_uuid_t service, simpleble_uuid_t characteristic,
    simpleble_uuid_t descriptor, uint8_t** data, size_t* data_length);

/**
 * @brief
 *
 * @param handle
 * @param service
 * @param characteristic
 * @param descriptor
 * @param data
 * @param data_length
 * @return simpleble_err_t
 */
SHARED_EXPORT simpleble_err_t CALLING_CONVENTION simpleble_peripheral_write_descriptor(
    simpleble_peripheral_t handle, simpleble_uuid_t service, simpleble_uuid_t characteristic,
    simpleble_uuid_t descriptor, const uint8_t* data, size_t data_length);

/**
 * @brief
 *
 * @param handle
 * @param callback
 * @return simpleble_err_t
 */
SHARED_EXPORT simpleble_err_t CALLING_CONVENTION simpleble_peripheral_set_callback_on_connected(
    simpleble_peripheral_t handle, void (*callback)(simpleble_peripheral_t peripheral, void* userdata), void* userdata);

/**
 * @brief
 *
 * @param handle
 * @param callback
 * @return simpleble_err_t
 */
SHARED_EXPORT simpleble_err_t CALLING_CONVENTION simpleble_peripheral_set_callback_on_disconnected(
    simpleble_peripheral_t handle, void (*callback)(simpleble_peripheral_t peripheral, void* userdata), void* userdata);

#ifdef __cplusplus
}
#endif
