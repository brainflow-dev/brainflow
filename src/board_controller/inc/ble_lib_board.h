#pragma once

#include <mutex>

#include "board.h"
#include "board_controller.h"
#include "runtime_dll_loader.h"
#include "simpleble_c/types.h"


class BLELibBoard : public Board
{

private:
    static DLLLoader *dll_loader;
    static std::mutex mutex;

protected:
    static bool init_dll_loader ();
    // common
    void simpleble_free (void *handle);
    // adapter
    size_t simpleble_adapter_get_count (void);
    simpleble_adapter_t simpleble_adapter_get_handle (size_t index);
    void simpleble_adapter_release_handle (simpleble_adapter_t handle);
    simpleble_err_t simpleble_adapter_scan_for (simpleble_adapter_t handle, int timeout_ms);
    simpleble_err_t simpleble_adapter_scan_start (simpleble_adapter_t handle);
    simpleble_err_t simpleble_adapter_scan_stop (simpleble_adapter_t handle);
    simpleble_err_t simpleble_adapter_set_callback_on_scan_updated (simpleble_adapter_t handle,
        void (*) (simpleble_adapter_t, simpleble_peripheral_t, void *), void *);
    simpleble_err_t simpleble_adapter_set_callback_on_scan_found (simpleble_adapter_t handle,
        void (*) (simpleble_adapter_t, simpleble_peripheral_t, void *), void *);
    // peripheral
    char *simpleble_peripheral_address (simpleble_peripheral_t handle);
    char *simpleble_peripheral_identifier (simpleble_peripheral_t handle);
    void simpleble_peripheral_release_handle (simpleble_peripheral_t handle);
    simpleble_err_t simpleble_peripheral_connect (simpleble_peripheral_t handle);
    simpleble_err_t simpleble_peripheral_disconnect (simpleble_peripheral_t handle);
    size_t simpleble_peripheral_services_count (simpleble_peripheral_t handle);
    simpleble_err_t simpleble_peripheral_services_get (
        simpleble_peripheral_t handle, size_t index, simpleble_service_t *services);
    simpleble_err_t simpleble_peripheral_write_request (simpleble_peripheral_t handle,
        simpleble_uuid_t service, simpleble_uuid_t characteristic, uint8_t *data,
        size_t data_length);
    simpleble_err_t simpleble_peripheral_write_command (simpleble_peripheral_t handle,
        simpleble_uuid_t service, simpleble_uuid_t characteristic, uint8_t *data,
        size_t data_length);
    simpleble_err_t simpleble_peripheral_notify (simpleble_peripheral_t handle,
        simpleble_uuid_t service, simpleble_uuid_t characteristic,
        void (*) (simpleble_uuid_t, simpleble_uuid_t, uint8_t *, size_t, void *), void *);
    simpleble_err_t simpleble_peripheral_unsubscribe (
        simpleble_peripheral_t handle, simpleble_uuid_t service, simpleble_uuid_t characteristic);
    size_t simpleble_peripheral_manufacturer_data_count (simpleble_peripheral_t handle);
    simpleble_err_t simpleble_peripheral_manufacturer_data_get (simpleble_peripheral_t handle,
        size_t index, simpleble_manufacturer_data_t *manufacturer_data);
    simpleble_err_t simpleble_peripheral_is_connected (
        simpleble_peripheral_t handle, bool *connected);

public:
    BLELibBoard (int board_id, struct BrainFlowInputParams params);
    virtual ~BLELibBoard ();
};
