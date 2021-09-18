#pragma once

#include "board.h"
#include "board_controller.h"
#include "runtime_dll_loader.h"
#include "simpleble_c/types.h"


class BLELibBoard : public Board
{

private:
    static DLLLoader *dll_loader;
    static DLLLoader *get_dll_loader ();

protected:
    // adapter
    size_t simpleble_adapter_get_adapter_count (void);
    simpleble_adapter_t simpleble_adapter_get_adapter_handle (size_t index);
    simpleble_err_t simpleble_adapter_scan_for (simpleble_adapter_t handle,
        int timeout_ms) simpleble_err_t simpleble_adapter_scan_start (simpleble_adapter_t handle);
    simpleble_err_t simpleble_adapter_scan_stop (simpleble_adapter_t handle);
    simpleble_err_t simpleble_adapter_set_callback_on_scan_updated (simpleble_adapter_t handle,
        void (*callback) (simpleble_adapter_t adapter, simpleble_peripheral_t peripheral));
    // peripheral
    const char *simpleble_peripheral_address (simpleble_peripheral_t handle);
    simpleble_err_t simpleble_peripheral_connect (simpleble_peripheral_t handle);
    simpleble_err_t simpleble_peripheral_disconnect (simpleble_peripheral_t handle);
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
        void (*callback) (simpleble_uuid_t service, simpleble_uuid_t characteristic, uint8_t *data,
            size_t data_length));
    simpleble_err_t simpleble_peripheral_unsubscribe (
        simpleble_peripheral_t handle, simpleble_uuid_t service, simpleble_uuid_t characteristic);

public:
    BLELibBoard (int board_id, struct BrainFlowInputParams params);
    virtual ~BLELibBoard ();
};
