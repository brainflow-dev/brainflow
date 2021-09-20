#pragma once

#include <stddef.h>
#include <stdint.h>

#include <simpleble_c/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief
 *
 * @return size_t
 */
size_t simpleble_adapter_get_adapter_count(void);

/**
 * @brief
 *
 * @param index
 * @return simpleble_adapter_t
 */
simpleble_adapter_t simpleble_adapter_get_adapter_handle(size_t index);

/**
 * @brief Returns the identifier of a given adapter.
 * 
 * @note The user is responsible for freeing the returned value.
 *
 * @param handle
 * @return const char*
 */
const char* simpleble_adapter_identifier(simpleble_adapter_t handle);

/**
 * @brief Returns the MAC address of a given adapter.
 * 
 * @note The user is responsible for freeing the returned value.
 *
 * @param handle
 * @return const char*
 */
const char* simpleble_adapter_address(simpleble_adapter_t handle);

/**
 * @brief
 *
 * @param handle
 * @return simpleble_err_t
 */
simpleble_err_t simpleble_adapter_scan_start(simpleble_adapter_t handle);

/**
 * @brief
 *
 * @param handle
 * @return simpleble_err_t
 */
simpleble_err_t simpleble_adapter_scan_stop(simpleble_adapter_t handle);

/**
 * @brief
 *
 * @param handle
 * @param active
 * @return simpleble_err_t
 */
simpleble_err_t simpleble_adapter_scan_is_active(simpleble_adapter_t handle, bool* active);

/**
 * @brief
 *
 * @param handle
 * @param timeout_ms
 * @return simpleble_err_t
 */
simpleble_err_t simpleble_adapter_scan_for(simpleble_adapter_t handle, int timeout_ms);

/**
 * @brief
 *
 * @param handle
 * @return size_t
 */
size_t simpleble_adapter_scan_get_results_count(simpleble_adapter_t handle);

/**
 * @brief
 *
 * @param handle
 * @param index
 * @return simpleble_peripheral_t
 */
simpleble_peripheral_t simpleble_adapter_scan_get_results_handle(simpleble_adapter_t handle, size_t index);

/**
 * @brief
 *
 * @param handle
 * @param callback
 * @return simpleble_err_t
 */
simpleble_err_t simpleble_adapter_set_callback_on_scan_start(simpleble_adapter_t handle,
                                                             void (*callback)(simpleble_adapter_t adapter));

/**
 * @brief
 *
 * @param handle
 * @param callback
 * @return simpleble_err_t
 */
simpleble_err_t simpleble_adapter_set_callback_on_scan_stop(simpleble_adapter_t handle,
                                                            void (*callback)(simpleble_adapter_t adapter));

/**
 * @brief
 *
 * @param handle
 * @param callback
 * @return simpleble_err_t
 */
simpleble_err_t simpleble_adapter_set_callback_on_scan_updated(simpleble_adapter_t handle,
                                                               void (*callback)(simpleble_adapter_t adapter,
                                                                                simpleble_peripheral_t peripheral));

/**
 * @brief
 *
 * @param handle
 * @param callback
 * @return simpleble_err_t
 */
simpleble_err_t simpleble_adapter_set_callback_on_scan_found(simpleble_adapter_t handle,
                                                             void (*callback)(simpleble_adapter_t adapter,
                                                                              simpleble_peripheral_t peripheral));

#ifdef __cplusplus
}
#endif
