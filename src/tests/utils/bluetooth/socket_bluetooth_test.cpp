#include "bluetooth_test_definitions.h"
#include "bluetooth_types.h"
#include "socket_bluetooth.h"

SocketBluetooth::SocketBluetooth (std::string mac_addr, int port)
{
    this->mac_addr = mac_addr;
    this->port = port;
}

int SocketBluetooth::connect ()
{
    if (mac_addr != BRAINFLOW_TEST_BLUETOOTH_VALID_MAC ||
        port == BRAINFLOW_TEST_BLUETOOTH_CANNOT_OPEN_PORT)
    {
        return (int)SocketBluetoothReturnCodes::CONNECT_ERROR;
    }
    return (int)SocketBluetoothReturnCodes::STATUS_OK;
}

int SocketBluetooth::send (const char *data, int size)
{
    switch (port)
    {
        case BRAINFLOW_TEST_BLUETOOTH_ALL_BYTES_AVAILABLE_PORT:
            return size;
        case BRAINFLOW_TEST_BLUETOOTH_SOME_BYTES_AVAILABLE_PORT:
            return size / 2;
        default:
            return 0;
    }
}

int SocketBluetooth::recv (char *data, int size)
{
    if (data == NULL)
    {
        return 0;
    }

    int n_bytes = 0;
    switch (port)
    {
        case BRAINFLOW_TEST_BLUETOOTH_ALL_BYTES_AVAILABLE_PORT:
            n_bytes = BRAINFLOW_TEST_BLUETOOTH_EXPECTED_BYTES;
        case BRAINFLOW_TEST_BLUETOOTH_SOME_BYTES_AVAILABLE_PORT:
            n_bytes = BRAINFLOW_TEST_BLUETOOTH_EXPECTED_BYTES / 2;
        default:
            n_bytes = 0;
    }

    if (n_bytes >= size)
    {
        for (auto i = 0; i < size; i++)
        {
            data[i] = BRAINFLOW_TEST_BLUETOOTH_DATA_CHAR;
        }
        return size;
    }

    return 0;
}

int SocketBluetooth::close ()
{
    return port == BRAINFLOW_TEST_BLUETOOTH_CANNOT_CLOSE_PORT ?
        (int)SocketBluetoothReturnCodes::DISCONNECT_ERROR :
        (int)SocketBluetoothReturnCodes::STATUS_OK;
}

std::pair<std::string, int> SocketBluetooth::discover (char *device_selector)
{
    if ((device_selector == NULL) ||
        (strcmp (device_selector, BRAINFLOW_TEST_BLUETOOTH_DISCOVERABLE_SELECTOR) != 0))
    {
        return std::make_pair<std::string, int> (
            "", (int)SocketBluetoothReturnCodes::DEVICE_IS_NOT_DISCOVERABLE);
    }

    return std::make_pair<std::string, int> (
        BRAINFLOW_TEST_BLUETOOTH_VALID_MAC, (int)SocketBluetoothReturnCodes::STATUS_OK);
}