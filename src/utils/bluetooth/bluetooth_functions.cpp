#include <map>
#include <mutex>
#include <string>

#include "bluetooth_functions.h"
#include "socket_bluetooth.h"

std::map<std::string, std::shared_ptr<SocketBluetooth>> devices;
std::mutex mutex;

int bluetooth_open_device (int port, char *mac_addr)
{
    std::lock_guard<std::mutex> lock (mutex);
    std::string key = mac_addr;
    if (devices.find (key) != devices.end ())
    {
        return (int)SocketBluetoothReturnCodes::ANOTHER_DEVICE_IS_CREATED;
    }
    std::shared_ptr<SocketBluetooth> device =
        std::shared_ptr<SocketBluetooth> (new SocketBluetooth (key, port));
    int res = device->connect ();
    if (res != (int)SocketBluetoothReturnCodes::STATUS_OK)
    {
        device = NULL;
    }
    else
    {
        devices[key] = device;
    }
    return res;
}

int bluetooth_get_data (char *data, int size, char *mac_addr)
{
    std::lock_guard<std::mutex> lock (mutex);

    std::string key = mac_addr;
    auto device_it = devices.find (key);
    if (device_it == devices.end ())
    {
        return (int)SocketBluetoothReturnCodes::DEVICE_IS_NOT_CREATED;
    }
    return device_it->second->recv (data, size);
}

int bluetooth_write_data (char *data, int size, char *mac_addr)
{
    std::lock_guard<std::mutex> lock (mutex);

    std::string key = mac_addr;
    auto device_it = devices.find (key);
    if (device_it == devices.end ())
    {
        return (int)SocketBluetoothReturnCodes::DEVICE_IS_NOT_CREATED;
    }
    return device_it->second->send (data, size);
}

int bluetooth_close_device (char *mac_addr)
{
    std::lock_guard<std::mutex> lock (mutex);

    std::string key = mac_addr;
    auto device_it = devices.find (key);
    if (device_it == devices.end ())
    {
        return (int)SocketBluetoothReturnCodes::DEVICE_IS_NOT_CREATED;
    }
    int res = device_it->second->close ();
    devices.erase (device_it);
    return res;
}
