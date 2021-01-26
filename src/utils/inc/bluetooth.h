#pragma once

#include <functional>

enum class BluetoothExitCodes : int
{
    OK = 0,
    ADAPTER_OPEN = -1,
    ADAPTER_CLOSED = -2,
    ADAPTER_SCANNING = -3,
    ADAPTER_NOT_SCANNING = -4,
    DEVICE_CONNECTED = -5,
    DEVICE_NOT_CONNECTED = -6,
    CHARACTERISTIC_SUBSCRIBED = -7,
    CHARACTERISTIC_NOT_SUBSCRIBED = -8,
    INVALID_PARAMETER = -9,
    DRIVER_ERROR = -10,
};

class BluetoothLE
{
public:
    static BluetoothLE * create (const char *adapter_name = "");

    virtual ~BluetoothLE () = 0;

    virtual int open_bt_adapter () = 0;
    virtual bool is_bt_adapter_open () = 0;

    using new_device_cb = std::function<void (const char *address, const char *name)>;
    virtual int start_bt_scanning (new_device_cb on_new_device) = 0;
    virtual bool is_bt_scanning () = 0;
    virtual int stop_bt_scanning () = 0;

    virtual int connect_bt_dev (const char *address) = 0;
    virtual bool is_bt_dev_connected () = 0;

    using subscription_cb = std::function<void (const uint8_t *data, size_t len)>;
    virtual int write_bt_dev_characteristic (const char *service_uuid, const char *characteristic_uuid,
        bool wait_for_response, const uint8_t *data, size_t len) = 0;
    virtual int subscribe_bt_dev_characteristic (const char *service_uuid, const char *characteristic_uuid,
        bool notification, subscription_cb on_data) = 0;
    // bool is_bt_dev_characteristic_subscribed (const char *service_uuid, const char
    // *characteristic_uuid);
    virtual int unsubscribe_bt_dev_characteristic (
        const char *service_uuid, const char *characteristic_uuid) = 0;

    virtual int disconnect_bt_dev () = 0;
    virtual int close_bt_adapter () = 0;
};
