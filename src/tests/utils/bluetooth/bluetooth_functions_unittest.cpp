#include <gmock/gmock-matchers.h>
#include <gmock/gmock.h>

#include "bluetooth_functions.h"
#include "bluetooth_test_definitions.h"
#include "bluetooth_types.h"
#include "socket_bluetooth.h"

using namespace testing;

// Close devices or assert that they do not open. Otherwise there may be cross test pollution.

TEST (BluetoothTest, OpenDevice_DeviceCanOpen_SuccessCodeReturned)
{
    char mac[] = BRAINFLOW_TEST_BLUETOOTH_VALID_MAC;

    int result = bluetooth_open_device (BRAINFLOW_TEST_BLUETOOTH_VALID_PORT, mac);
    EXPECT_EQ (result, (int)SocketBluetoothReturnCodes::STATUS_OK);

    bluetooth_close_device (mac);
}

TEST (BluetoothTest, OpenDevice_DeviceFailedToOpen_ConnectErrorReturned)
{
    char mac[] = BRAINFLOW_TEST_BLUETOOTH_VALID_MAC;
    int result = bluetooth_open_device (BRAINFLOW_TEST_BLUETOOTH_CANNOT_OPEN_PORT, mac);

    ASSERT_EQ (result, (int)SocketBluetoothReturnCodes::CONNECT_ERROR);
}

TEST (BluetoothTest, OpenDevice_DeviceAlreadyOpen_AlreadyCreatedCodeReturned)
{
    char mac[] = BRAINFLOW_TEST_BLUETOOTH_VALID_MAC;
    bluetooth_open_device (BRAINFLOW_TEST_BLUETOOTH_VALID_PORT, mac);

    // Try and open the same device again
    int result = bluetooth_open_device (BRAINFLOW_TEST_BLUETOOTH_VALID_PORT, mac);

    EXPECT_EQ (result, (int)SocketBluetoothReturnCodes::ANOTHER_DEVICE_IS_CREATED_ERROR);

    bluetooth_close_device (mac);
}

TEST (BluetoothTest, OpenDevice_AddressNull_ParameterErrorReturned)
{
    int result = bluetooth_open_device (BRAINFLOW_TEST_BLUETOOTH_VALID_PORT, NULL);
    ASSERT_EQ (result, (int)SocketBluetoothReturnCodes::PARAMETER_ERROR);
}

TEST (BluetoothTest, OpenDevice_NegativePort_ParameterErrorReturned)
{
    char mac[] = BRAINFLOW_TEST_BLUETOOTH_VALID_MAC;
    int result = bluetooth_open_device (-1, mac);
    ASSERT_EQ (result, (int)SocketBluetoothReturnCodes::PARAMETER_ERROR);
}

TEST (BluetoothTest, CloseDevice_DeviceCanClose_SuccessCodeReturned)
{
    char mac[] = BRAINFLOW_TEST_BLUETOOTH_VALID_MAC;
    bluetooth_open_device (BRAINFLOW_TEST_BLUETOOTH_VALID_PORT, mac);

    int result = bluetooth_close_device (mac);
    EXPECT_EQ (result, (int)SocketBluetoothReturnCodes::STATUS_OK);
}

TEST (BluetoothTest, CloseDevice_DeviceCannotClose_DisconnectErrorReturned)
{
    char mac[] = BRAINFLOW_TEST_BLUETOOTH_VALID_MAC;

    bluetooth_open_device (BRAINFLOW_TEST_BLUETOOTH_CANNOT_CLOSE_PORT, mac);
    int result = bluetooth_close_device (mac);

    EXPECT_EQ (result, (int)SocketBluetoothReturnCodes::DISCONNECT_ERROR);
}

TEST (BluetoothTest, CloseDevice_DeviceNotCreated_DeviceNotCreatedErrorReturned)
{
    char mac[] = BRAINFLOW_TEST_BLUETOOTH_VALID_MAC;
    int result = bluetooth_close_device (mac);

    EXPECT_EQ (result, (int)SocketBluetoothReturnCodes::DEVICE_IS_NOT_CREATED_ERROR);
}

TEST (BluetoothTest, CloseDevice_AddressNull_ParameterErrorReturned)
{
    int result = bluetooth_close_device (NULL);

    EXPECT_EQ (result, (int)SocketBluetoothReturnCodes::PARAMETER_ERROR);
}

TEST (BluetoothTest, GetData_SomeRequestedBytesAvailable_NumberBytesReadReturned)
{
    char mac[] = BRAINFLOW_TEST_BLUETOOTH_VALID_MAC;
    bluetooth_open_device (BRAINFLOW_TEST_BLUETOOTH_SOME_BYTES_AVAILABLE_PORT, mac);

    char buffer[BRAINFLOW_TEST_BLUETOOTH_EXPECTED_BYTES] {0};
    int n_bytes = bluetooth_get_data (buffer, BRAINFLOW_TEST_BLUETOOTH_EXPECTED_BYTES, mac);

    EXPECT_EQ (n_bytes, 0);
    bluetooth_close_device (mac);
}

TEST (BluetoothTest, GetData_NoBytesAvailable_NoBytesReturned)
{
    char mac[] = BRAINFLOW_TEST_BLUETOOTH_VALID_MAC;
    bluetooth_open_device (BRAINFLOW_TEST_BLUETOOTH_NO_BYTES_AVAILABLE_PORT, mac);

    char buffer[BRAINFLOW_TEST_BLUETOOTH_EXPECTED_BYTES] {0};
    int n_bytes = bluetooth_get_data (buffer, BRAINFLOW_TEST_BLUETOOTH_EXPECTED_BYTES, mac);

    EXPECT_EQ (n_bytes, 0);
    EXPECT_THAT (buffer, Contains (BRAINFLOW_TEST_BLUETOOTH_DATA_CHAR).Times (0));

    bluetooth_close_device (mac);
}

TEST (BluetoothTest, GetData_NegativeSize_ParameterErrorReturned)
{
    char mac[] = BRAINFLOW_TEST_BLUETOOTH_VALID_MAC;
    bluetooth_open_device (BRAINFLOW_TEST_BLUETOOTH_ALL_BYTES_AVAILABLE_PORT, mac);

    char buffer[BRAINFLOW_TEST_BLUETOOTH_EXPECTED_BYTES] {0};
    int n_bytes = bluetooth_get_data (buffer, -1, mac);

    EXPECT_EQ (n_bytes, (int)SocketBluetoothReturnCodes::PARAMETER_ERROR);

    bluetooth_close_device (mac);
}

TEST (BluetoothTest, GetData_AddressNull_ParameterErrorReturned)
{
    char mac[] = BRAINFLOW_TEST_BLUETOOTH_VALID_MAC;
    bluetooth_open_device (BRAINFLOW_TEST_BLUETOOTH_ALL_BYTES_AVAILABLE_PORT, mac);

    char buffer[BRAINFLOW_TEST_BLUETOOTH_EXPECTED_BYTES] {0};
    int n_bytes = bluetooth_get_data (buffer, BRAINFLOW_TEST_BLUETOOTH_EXPECTED_BYTES, NULL);

    EXPECT_EQ (n_bytes, (int)SocketBluetoothReturnCodes::PARAMETER_ERROR);

    bluetooth_close_device (mac);
}

TEST (BluetoothTest, GetData_NullDataBuffer_ParameterErrorReturned)
{
    char mac[] = BRAINFLOW_TEST_BLUETOOTH_VALID_MAC;
    bluetooth_open_device (BRAINFLOW_TEST_BLUETOOTH_ALL_BYTES_AVAILABLE_PORT, mac);

    int n_bytes = bluetooth_get_data (NULL, BRAINFLOW_TEST_BLUETOOTH_EXPECTED_BYTES, mac);
    EXPECT_EQ (n_bytes, (int)SocketBluetoothReturnCodes::PARAMETER_ERROR);

    bluetooth_close_device (mac);
}

TEST (BluetoothTest, GetData_DeviceNotCreated_DeviceNotCreatedErrorReturned)
{
    char mac[] = BRAINFLOW_TEST_BLUETOOTH_VALID_MAC;

    char buffer[BRAINFLOW_TEST_BLUETOOTH_EXPECTED_BYTES] {0};
    int n_bytes = bluetooth_get_data (buffer, BRAINFLOW_TEST_BLUETOOTH_EXPECTED_BYTES, mac);

    EXPECT_EQ (n_bytes, (int)SocketBluetoothReturnCodes::DEVICE_IS_NOT_CREATED_ERROR);
}

TEST (BluetoothTest, WriteData_AllBytesCanBeWritten_NumberOfProvidedBytesReturned)
{
    char mac[] = BRAINFLOW_TEST_BLUETOOTH_VALID_MAC;
    bluetooth_open_device (BRAINFLOW_TEST_BLUETOOTH_ALL_BYTES_AVAILABLE_PORT, mac);

    char buffer[8] {'a'};
    int n_bytes = bluetooth_write_data (buffer, 8, mac);

    EXPECT_EQ (n_bytes, 8);

    bluetooth_close_device (mac);
}

TEST (BluetoothTest, WriteData_SomeBytesCanBeWritten_NumberBytesWrittenReturned)
{
    char mac[] = BRAINFLOW_TEST_BLUETOOTH_VALID_MAC;
    bluetooth_open_device (BRAINFLOW_TEST_BLUETOOTH_SOME_BYTES_AVAILABLE_PORT, mac);

    char buffer[8] {'a'};
    int n_bytes = bluetooth_write_data (buffer, 8, mac);

    EXPECT_GT (n_bytes, 0);
    EXPECT_LT (n_bytes, 8);

    bluetooth_close_device (mac);
}

TEST (BluetoothTest, WriteData_NoBytesCanBeWritten_ZeroReturned)
{
    char mac[] = BRAINFLOW_TEST_BLUETOOTH_VALID_MAC;
    bluetooth_open_device (BRAINFLOW_TEST_BLUETOOTH_NO_BYTES_AVAILABLE_PORT, mac);

    char buffer[8] {'a'};
    int n_bytes = bluetooth_write_data (buffer, 8, mac);

    EXPECT_EQ (n_bytes, 0);

    bluetooth_close_device (mac);
}

TEST (BluetoothTest, WriteData_NegativeSize_ParameterErrorReturned)
{
    char mac[] = BRAINFLOW_TEST_BLUETOOTH_VALID_MAC;
    bluetooth_open_device (BRAINFLOW_TEST_BLUETOOTH_ALL_BYTES_AVAILABLE_PORT, mac);

    char buffer[8] {'a'};
    int n_bytes = bluetooth_write_data (buffer, -1, mac);

    EXPECT_EQ (n_bytes, (int)SocketBluetoothReturnCodes::PARAMETER_ERROR);

    bluetooth_close_device (mac);
}

TEST (BluetoothTest, WriteData_AddressNull_ParameterErrorReturned)
{
    char mac[] = BRAINFLOW_TEST_BLUETOOTH_VALID_MAC;
    bluetooth_open_device (BRAINFLOW_TEST_BLUETOOTH_ALL_BYTES_AVAILABLE_PORT, mac);

    char buffer[8] {'a'};
    int n_bytes = bluetooth_write_data (buffer, 8, NULL);

    EXPECT_EQ (n_bytes, (int)SocketBluetoothReturnCodes::PARAMETER_ERROR);

    bluetooth_close_device (mac);
}

TEST (BluetoothTest, WriteData_NullDataBuffer_ParameterErrorReturned)
{
    char mac[] = BRAINFLOW_TEST_BLUETOOTH_VALID_MAC;
    bluetooth_open_device (BRAINFLOW_TEST_BLUETOOTH_ALL_BYTES_AVAILABLE_PORT, mac);

    int n_bytes = bluetooth_write_data (NULL, 8, mac);

    EXPECT_EQ (n_bytes, (int)SocketBluetoothReturnCodes::PARAMETER_ERROR);

    bluetooth_close_device (mac);
}

TEST (BluetoothTest, WriteData_DeviceNotCreated_DeviceNotCreatedErrorReturned)
{
    char mac[] = BRAINFLOW_TEST_BLUETOOTH_VALID_MAC;

    char buffer[8] {'a'};
    int n_bytes = bluetooth_write_data (buffer, 8, mac);

    EXPECT_EQ (n_bytes, (int)SocketBluetoothReturnCodes::DEVICE_IS_NOT_CREATED_ERROR);
}

TEST (BluetoothTest, Discover_DeviceIsDiscoverable_AddressAndSuccessCodeReturned)
{
    int length;
    char mac[32]; // MAC can be stored in 6 bytes, but oversize in case formatting makes it bigger
    char selector[] = BRAINFLOW_TEST_BLUETOOTH_DISCOVERABLE_SELECTOR;

    int result = bluetooth_discover_device (selector, mac, &length);

    EXPECT_EQ (result, (int)SocketBluetoothReturnCodes::STATUS_OK);
    EXPECT_STREQ (mac, BRAINFLOW_TEST_BLUETOOTH_VALID_MAC);
    EXPECT_EQ (length, strlen (BRAINFLOW_TEST_BLUETOOTH_VALID_MAC));
}

TEST (BluetoothTest, Discover_DeviceIsNotDiscoverable_DeviceNotDiscoverableErrorReturned)
{
    int length;
    char mac[32];
    char selector[] = BRAINFLOW_TEST_BLUETOOTH_NOT_DISCOVERABLE_SELECTOR;

    int result = bluetooth_discover_device (selector, mac, &length);

    EXPECT_EQ (result, (int)SocketBluetoothReturnCodes::DEVICE_IS_NOT_DISCOVERABLE);
}

TEST (BluetoothTest, Discover_SelectorNull_ParameterErrorReturned)
{
    int length;
    char mac[32];

    int result = bluetooth_discover_device (NULL, mac, &length);

    EXPECT_EQ (result, (int)SocketBluetoothReturnCodes::PARAMETER_ERROR);
}

TEST (BluetoothTest, Discover_AddressNull_ParameterErrorReturned)
{
    int length;
    char selector[] = BRAINFLOW_TEST_BLUETOOTH_NOT_DISCOVERABLE_SELECTOR;

    int result = bluetooth_discover_device (selector, NULL, &length);

    EXPECT_EQ (result, (int)SocketBluetoothReturnCodes::PARAMETER_ERROR);
}

TEST (BluetoothTest, Discover_LengthPointerNull_ParameterErrorReturned)
{
    char mac[32];
    char selector[] = BRAINFLOW_TEST_BLUETOOTH_NOT_DISCOVERABLE_SELECTOR;

    int result = bluetooth_discover_device (selector, mac, NULL);

    EXPECT_EQ (result, (int)SocketBluetoothReturnCodes::PARAMETER_ERROR);
}