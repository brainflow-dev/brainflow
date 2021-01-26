#include "bluetooth.h"

#include <future>
#include <iostream>

#define BACKEND_NATIVE_BLE 1
#define BACKEND_GATTLIB 2

#define BT_BACKEND BACKEND_NATIVE_BLE


/////////////////////////////////////////////////
/////////////////// gattlib /////////////////////
/////////////////////////////////////////////////

#if BT_BACKEND == BACKEND_GATTLIB

// note: gattlib can notify on disconnection via gattlib_register_on_disconnect
// this feature is not used at this time [to add it, add an interface to inc/bluetooth.h]

// note 2: the gattlib driver was hacked together in a very rushed manner to compare against
// the nativeble driver.  it may need changes to be stable, reliable, and clear.

#include <gattlib.h>
#include <glib.h>

// this preprocessor define is used extensively
#define gldata static_cast<GattlibData *> (this->backend)

// gattlib uses bluez uuid_t structures to pass uuids
// they are unions based off the bluetooth specification
namespace std
{
    template <> struct hash<uuid_t>
    {
        inline size_t operator() (const uuid_t &uuid) const
        {
            return std::hash<uint16_t> () (uuid.value.uuid16);
        }
    };
    template <> struct equal_to<uuid_t>
    {
        inline size_t operator() (const uuid_t &lhs, const uuid_t &rhs) const
        {
            return gattlib_uuid_cmp (&lhs, &rhs) == GATTLIB_SUCCESS;
        }
    };
}

struct gmainloop
{
    gmainloop () : loop (g_main_loop_new (nullptr, 0)), thread (std::thread (g_main_loop_run, loop))
    {
    }

    ~gmainloop ()
    {
        g_main_loop_quit (loop);
        thread.join ();
        g_main_loop_unref (loop);
    }

    GMainLoop *loop;
    std::thread thread;

    static std::weak_ptr<gmainloop> singleton;
    static std::shared_ptr<gmainloop> get ()
    {
        // TODO: needs lock (but really, couldn't this be more organised)
        if (singleton.expired ())
        {
            auto result = std::make_shared<gmainloop> ();
            singleton = result;
            return result;
        }
        else
        {
            return singleton.lock ();
        }
    }
};
std::weak_ptr<gmainloop> gmainloop::singleton;

struct GattlibData
{
    std::string adapter_name;
    void *adapter;
    BluetoothLE::new_device_cb scan_callback;
    std::thread scan_thread;
    gatt_connection_t *connection;
    std::unordered_map<uuid_t, std::pair<bool, BluetoothLE::subscription_cb>> data_callbacks;
    std::shared_ptr<gmainloop> loop;

    static int code (int bt_gattlib_error)
    {
        switch (bt_gattlib_error)
        {
            case GATTLIB_SUCCESS:
                return (int)BluetoothExitCodes::OK;
            case GATTLIB_INVALID_PARAMETER:
            case GATTLIB_NOT_FOUND:
            case GATTLIB_NOT_SUPPORTED:
                return (int)BluetoothExitCodes::INVALID_PARAMETER;
            default:
                return (int)BluetoothExitCodes::DRIVER_ERROR;
        }
    }

    static void on_new_device (void *adapter, const char *address, const char *name, void *self)
    {
        auto &scan_callback = static_cast<GattlibData *> (self)->scan_callback;
        if (name == nullptr)
        {
            name = "";
        }
        scan_callback (address, name);
    }

    static void on_data (const uuid_t *uuid, const uint8_t *data, size_t data_length, void *self)
    {
        std::cerr << "data callback" << std::endl;
        auto &data_callbacks = static_cast<GattlibData *> (self)->data_callbacks;

        auto cb_iter = data_callbacks.find (*uuid);
        if (cb_iter != data_callbacks.end ())
        {
            cb_iter->second.second (data, data_length);
        }
    }
};

BluetoothLE::BluetoothLE (const char *adapter_name) : backend (new GattlibData ())
{
    if (adapter_name != nullptr)
    {
        gldata->adapter_name = adapter_name;
    }
    gldata->adapter = nullptr;
    gldata->loop = gmainloop::get ();
}

BluetoothLE::~BluetoothLE ()
{
    close_bt_adapter ();
    delete gldata;
}

int BluetoothLE::open_bt_adapter ()
{
    gldata->connection = nullptr;

    int error_code = gattlib_adapter_open (
        gldata->adapter_name.empty () ? nullptr : gldata->adapter_name.c_str (), &gldata->adapter);
    return GattlibData::code (error_code);
}

bool BluetoothLE::is_bt_adapter_open ()
{
    return gldata->adapter != nullptr;
}

int BluetoothLE::start_bt_scanning (new_device_cb on_new_device)
{
    if (!is_bt_adapter_open ())
    {
        return (int)BluetoothExitCodes::ADAPTER_CLOSED;
    }

    if (is_bt_scanning ())
    {
        return (int)BluetoothExitCodes::ADAPTER_SCANNING;
    }

    gldata->scan_callback = on_new_device;
    gldata->scan_thread = std::thread (
        gattlib_adapter_scan_enable, gldata->adapter, GattlibData::on_new_device, 0, gldata);

    return (int)BluetoothExitCodes::OK;
}

bool BluetoothLE::is_bt_scanning ()
{
    if (!is_bt_adapter_open ())
    {
        return false;
    }

    return gldata->scan_thread.joinable ();
}

int BluetoothLE::stop_bt_scanning ()
{
    if (!is_bt_adapter_open ())
    {
        return (int)BluetoothExitCodes::ADAPTER_CLOSED;
    }

    if (!is_bt_scanning ())
    {
        return (int)BluetoothExitCodes::ADAPTER_NOT_SCANNING;
    }

    int error_code = gattlib_adapter_scan_disable (gldata->adapter);
    if (error_code == GATTLIB_SUCCESS)
    {
        gldata->scan_thread.join ();
    }
    return GattlibData::code (error_code);
}

int BluetoothLE::connect_bt_dev (const char *address)
{
    if (!is_bt_adapter_open ())
    {
        return (int)BluetoothExitCodes::ADAPTER_CLOSED;
    }
    if (is_bt_dev_connected ())
    {
        return (int)BluetoothExitCodes::DEVICE_CONNECTED;
    }

    gldata->connection =
        gattlib_connect (gldata->adapter, address, GATTLIB_CONNECTION_OPTIONS_LEGACY_DEFAULT);

    if (gldata->connection == nullptr)
    {
        return (int)BluetoothExitCodes::DEVICE_NOT_CONNECTED;
    }

    gattlib_register_notification (gldata->connection, GattlibData::on_data, gldata);
    gattlib_register_indication (gldata->connection, GattlibData::on_data, gldata);

    return (int)BluetoothExitCodes::OK;
}

bool BluetoothLE::is_bt_dev_connected ()
{
    if (!is_bt_adapter_open ())
    {
        return false;
    }

    return gldata->connection != nullptr;
}

int BluetoothLE::write_bt_dev_characteristic (const char *service_uuid,
    const char *characteristic_uuid, bool wait_for_response, const uint8_t *data, size_t len)
{
    if (!is_bt_dev_connected ())
    {
        return (int)BluetoothExitCodes::DEVICE_NOT_CONNECTED;
    }

    uuid_t uuid;
    int error_code;

    error_code = gattlib_string_to_uuid (characteristic_uuid, strlen (characteristic_uuid), &uuid);
    if (error_code != GATTLIB_SUCCESS)
    {
        return GattlibData::code (error_code);
    }

    if (wait_for_response)
    {
        error_code = gattlib_write_char_by_uuid (gldata->connection, &uuid, data, len);
    }
    else
    {
        error_code =
            gattlib_write_without_response_char_by_uuid (gldata->connection, &uuid, data, len);
    }

    return GattlibData::code (error_code);
}

int BluetoothLE::subscribe_bt_dev_characteristic (const char *service_uuid,
    const char *characteristic_uuid, bool notification, subscription_cb on_data)
{
    if (!is_bt_dev_connected ())
    {
        return (int)BluetoothExitCodes::DEVICE_NOT_CONNECTED;
    }

    int error_code;
    uuid_t uuid;

    error_code = gattlib_string_to_uuid (characteristic_uuid, strlen (characteristic_uuid), &uuid);
    if (error_code != GATTLIB_SUCCESS)
    {
        return GattlibData::code (error_code);
    }

    gldata->data_callbacks[uuid] = std::make_pair (notification, on_data);

    std::cerr << "subscribing" << std::endl;
    if (notification)
    {
        error_code = gattlib_notification_start (gldata->connection, &uuid);
    }
    else
    {
        error_code = gattlib_indication_start (gldata->connection, &uuid);
    }

    return GattlibData::code (error_code);
}

int BluetoothLE::unsubscribe_bt_dev_characteristic (
    const char *service_uuid, const char *characteristic_uuid)
{
    if (!is_bt_dev_connected ())
    {
        return (int)BluetoothExitCodes::DEVICE_NOT_CONNECTED;
    }

    int error_code;
    uuid_t uuid;

    error_code = gattlib_string_to_uuid (characteristic_uuid, strlen (characteristic_uuid), &uuid);
    if (error_code != GATTLIB_SUCCESS)
    {
        return GattlibData::code (error_code);
    }

    auto entry = gldata->data_callbacks.find (uuid);
    if (entry == gldata->data_callbacks.end ())
    {
        return (int)BluetoothExitCodes::CHARACTERISTIC_NOT_SUBSCRIBED;
    }

    bool notification = entry->second.first;
    if (notification)
    {
        error_code = gattlib_notification_stop (gldata->connection, &uuid);
    }
    else
    {
        error_code = gattlib_indication_stop (gldata->connection, &uuid);
    }
    if (error_code != GATTLIB_SUCCESS)
    {
        return GattlibData::code (error_code);
    }

    gldata->data_callbacks.erase (uuid);

    return (int)BluetoothExitCodes::OK;
}

int BluetoothLE::disconnect_bt_dev ()
{
    if (!is_bt_adapter_open ())
    {
        return (int)BluetoothExitCodes::ADAPTER_CLOSED;
    }
    if (!is_bt_dev_connected ())
    {
        return (int)BluetoothExitCodes::DEVICE_NOT_CONNECTED;
    }

    int error_code = gattlib_disconnect (gldata->connection);
    if (error_code == GATTLIB_SUCCESS)
    {
        gldata->connection = nullptr;
    }
    return GattlibData::code (error_code);
}

int BluetoothLE::close_bt_adapter ()
{
    if (!is_bt_adapter_open ())
    {
        return (int)BluetoothExitCodes::ADAPTER_CLOSED;
    }

    int error_code = gattlib_adapter_close (gldata->adapter);
    if (error_code == GATTLIB_SUCCESS)
    {
        gldata->adapter = nullptr;
    }
    return GattlibData::code (error_code);
}

#endif // gattlib

/////////////////////////////////////////////////
/////////////////// NativeBLE ///////////////////
/////////////////////////////////////////////////

#if BT_BACKEND == BACKEND_NATIVE_BLE

#include <NativeBleController.h>

using namespace NativeBLE;

// this preprocessor define is used extensively
#define nativeble static_cast<NativeBleController *> (this->backend)

BluetoothLE::BluetoothLE (const char *adapter_name) : backend (nullptr)
{
}

BluetoothLE::~BluetoothLE ()
{
    close_bt_adapter ();
}

int BluetoothLE::open_bt_adapter ()
{
    if (is_bt_adapter_open ())
    {
        return (int)BluetoothExitCodes::ADAPTER_OPEN;
    }

    backend = new NativeBleController ();

    return (int)BluetoothExitCodes::OK;
}

bool BluetoothLE::is_bt_adapter_open ()
{
    return nativeble != nullptr;
}

int BluetoothLE::start_bt_scanning (new_device_cb on_new_device)
{
    if (!is_bt_adapter_open ())
    {
        return (int)BluetoothExitCodes::ADAPTER_CLOSED;
    }
    if (is_bt_scanning ())
    {
        return (int)BluetoothExitCodes::ADAPTER_SCANNING;
    }

    CallbackHolder callbacks;
    callbacks.callback_on_scan_found = [on_new_device] (DeviceDescriptor device) {
        on_new_device (device.address.c_str (), device.name.c_str ());
    };

    nativeble->setup (callbacks);
    nativeble->scan_start ();

    return (int)BluetoothExitCodes::OK;
}

bool BluetoothLE::is_bt_scanning ()
{
    if (!is_bt_adapter_open ())
    {
        return false;
    }

    return nativeble->scan_is_active ();
}

int BluetoothLE::stop_bt_scanning ()
{
    if (!is_bt_adapter_open ())
    {
        return (int)BluetoothExitCodes::ADAPTER_CLOSED;
    }
    if (!is_bt_scanning ())
    {
        return (int)BluetoothExitCodes::ADAPTER_NOT_SCANNING;
    }

    nativeble->scan_stop ();

    return (int)BluetoothExitCodes::OK;
}

int BluetoothLE::connect_bt_dev (const char *address)
{
    if (!is_bt_adapter_open ())
    {
        return (int)BluetoothExitCodes::ADAPTER_CLOSED;
    }
    if (is_bt_dev_connected ())
    {
        return (int)BluetoothExitCodes::DEVICE_CONNECTED;
    }

    nativeble->connect (address);

    if (!is_bt_dev_connected ())
    {
        return (int)BluetoothExitCodes::DEVICE_NOT_CONNECTED;
    }

    return (int)BluetoothExitCodes::OK;
}

bool BluetoothLE::is_bt_dev_connected ()
{
    if (!is_bt_adapter_open ())
    {
        return false;
    }

    return nativeble->is_connected ();
}

int BluetoothLE::write_bt_dev_characteristic (const char *service_uuid,
    const char *characteristic_uuid, bool wait_for_response, const uint8_t *data, size_t len)
{
    if (!is_bt_adapter_open ())
    {
        return (int)BluetoothExitCodes::ADAPTER_CLOSED;
    }
    if (!is_bt_dev_connected ())
    {
        return (int)BluetoothExitCodes::DEVICE_NOT_CONNECTED;
    }

    DataChunk datachunk (reinterpret_cast<const char *> (data), len);

    if (wait_for_response)
    {
        nativeble->write_request (service_uuid, characteristic_uuid, datachunk);
    }
    else
    {
        nativeble->write_command (service_uuid, characteristic_uuid, datachunk);
    }

    return (int)BluetoothExitCodes::OK;
}


int BluetoothLE::subscribe_bt_dev_characteristic (const char *service_uuid,
    const char *characteristic_uuid, bool notification, subscription_cb on_data)
{
    if (!is_bt_adapter_open ())
    {
        return (int)BluetoothExitCodes::ADAPTER_CLOSED;
    }
    if (!is_bt_dev_connected ())
    {
        return (int)BluetoothExitCodes::DEVICE_NOT_CONNECTED;
    }

    if (notification)
    {
        // if converting between size_t and uint32_t produces an error on some platform,
        // then wrap on_data with a lambda when passing it, like this:
        //   [on_data] (const uint8_t *data, uint32_t length) { on_data (data, length); }
        nativeble->notify (service_uuid, characteristic_uuid, on_data);
    }
    else
    {
        nativeble->indicate (service_uuid, characteristic_uuid, on_data);
    }

    return (int)BluetoothExitCodes::OK;
}

int BluetoothLE::unsubscribe_bt_dev_characteristic (
    const char *service_uuid, const char *characteristic_uuid)
{
    if (!is_bt_adapter_open ())
    {
        return (int)BluetoothExitCodes::ADAPTER_CLOSED;
    }
    if (!is_bt_dev_connected ())
    {
        return (int)BluetoothExitCodes::DEVICE_NOT_CONNECTED;
    }

    nativeble->unsubscribe (service_uuid, characteristic_uuid);

    return (int)BluetoothExitCodes::OK;
}

int BluetoothLE::disconnect_bt_dev ()
{
    if (!is_bt_adapter_open ())
    {
        return (int)BluetoothExitCodes::ADAPTER_CLOSED;
    }
    if (!is_bt_dev_connected ())
    {
        return (int)BluetoothExitCodes::DEVICE_NOT_CONNECTED;
    }

    nativeble->disconnect ();

    return (int)BluetoothExitCodes::OK;
}

int BluetoothLE::close_bt_adapter ()
{
    if (!is_bt_adapter_open ())
    {
        return (int)BluetoothExitCodes::ADAPTER_CLOSED;
    }

    if (is_bt_dev_connected ())
    {
        disconnect_bt_dev ();
    }

    nativeble->dispose ();

    delete nativeble;

    backend = nullptr;

    return (int)BluetoothExitCodes::OK;
}

#endif // NativeBLE
