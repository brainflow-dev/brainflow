#include <gmock/gmock.h>
#include <gmock/gmock-matchers.h>

#include "socket_bluetooth.h"
#include "bluetooth_functions.h"
#include "bluetooth_types.h"
#include "bluetooth_test_definitions.h"

using namespace testing;

// Close devices or assert that they do not open. Otherwise there may be cross test pollution.

TEST (BluetoothTest, OpenDevice_DeviceCanOpen_SuccessCodeReturned)
{
    char mac[] = BRAINFLOW_TEST_BLUETOOTH_VALID_MAC;

    int result = bluetooth_open_device (BRAINFLOW_TEST_BLUETOOTH_VALID_PORT, mac);
    EXPECT_EQ (result, (int)SocketBluetoothReturnCodes::STATUS_OK);

    bluetooth_close_device (mac);
}

TEST (BluetoothTest, OpenDevice_DeviceFailedToOpen_SuccessCodeNotReturned)
{
    char mac[] = BRAINFLOW_TEST_BLUETOOTH_VALID_MAC;
    int result = bluetooth_open_device (BRAINFLOW_TEST_BLUETOOTH_CANNOT_OPEN_PORT, mac);

    ASSERT_NE (result, (int)SocketBluetoothReturnCodes::STATUS_OK);
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

TEST (BluetoothTest, OpenDevice_AddressIsNull_ErrorReturned)
{
    int result = bluetooth_open_device (BRAINFLOW_TEST_BLUETOOTH_VALID_PORT, nullptr);
    ASSERT_EQ (result, (int)SocketBluetoothReturnCodes::GENERAL_ERROR);
}

TEST (BluetoothTest, OpenDevice_NegativePort_ErrorReturned)
{
    char mac[] = BRAINFLOW_TEST_BLUETOOTH_VALID_MAC;
    int result = bluetooth_open_device (-1, mac);
    ASSERT_EQ (result, (int)SocketBluetoothReturnCodes::GENERAL_ERROR);
}

TEST (BluetoothTest, CloseDevice_DeviceCanClose_SuccessCodeReturned)
{
    char mac[] = BRAINFLOW_TEST_BLUETOOTH_VALID_MAC;
    bluetooth_open_device (BRAINFLOW_TEST_BLUETOOTH_VALID_PORT, mac);

    int result = bluetooth_close_device (mac);
    EXPECT_EQ (result, (int)SocketBluetoothReturnCodes::STATUS_OK);
}

TEST (BluetoothTest, CloseDevice_DeviceCannotClose_ErrorReturned)
{
    char mac[] = BRAINFLOW_TEST_BLUETOOTH_VALID_MAC;

    bluetooth_open_device (BRAINFLOW_TEST_BLUETOOTH_CANNOT_CLOSE_PORT, mac);
    int result = bluetooth_close_device (mac);

    EXPECT_EQ (result, (int)SocketBluetoothReturnCodes::GENERAL_ERROR);
}

TEST (BluetoothTest, CloseDevice_DeviceNotCreated_DeviceNotCreatedErrorReturned)
{
    char mac[] = BRAINFLOW_TEST_BLUETOOTH_VALID_MAC;
    int result = bluetooth_close_device (mac);

    EXPECT_EQ (result, (int)SocketBluetoothReturnCodes::DEVICE_IS_NOT_CREATED_ERROR);
}

TEST (BluetoothTest, GetData_AllRequestedBytesAvailable_NumberOfRequestedBytesReturned)
{
    char mac[] = BRAINFLOW_TEST_BLUETOOTH_VALID_MAC;
    bluetooth_open_device (BRAINFLOW_TEST_BLUETOOTH_ALL_BYTES_AVAILABLE_PORT, mac);

    char buffer[BRAINFLOW_TEST_BLUETOOTH_EXPECTED_BYTES]{0};
    int n_bytes = bluetooth_get_data (buffer, BRAINFLOW_TEST_BLUETOOTH_EXPECTED_BYTES, mac);

    EXPECT_EQ (n_bytes, BRAINFLOW_TEST_BLUETOOTH_EXPECTED_BYTES);
    EXPECT_THAT (buffer, Each (BRAINFLOW_TEST_BLUETOOTH_DATA_CHAR));

    bluetooth_close_device (mac);
}

TEST (BluetoothTest, GetData_SomeRequestedBytesAvailable_NumberBytesReadReturned)
{
    char mac[] = BRAINFLOW_TEST_BLUETOOTH_VALID_MAC;
    bluetooth_open_device (BRAINFLOW_TEST_BLUETOOTH_SOME_BYTES_AVAILABLE_PORT, mac);

    char buffer[BRAINFLOW_TEST_BLUETOOTH_EXPECTED_BYTES]{0};
    int n_bytes = bluetooth_get_data (buffer, BRAINFLOW_TEST_BLUETOOTH_EXPECTED_BYTES, mac);

    EXPECT_GT (n_bytes, 0);
    EXPECT_LT (n_bytes, BRAINFLOW_TEST_BLUETOOTH_EXPECTED_BYTES);

    EXPECT_THAT (buffer, Contains (BRAINFLOW_TEST_BLUETOOTH_DATA_CHAR).Times (n_bytes));

    bluetooth_close_device (mac);
}

TEST (BluetoothTest, GetData_NoBytesAvailable_NoBytesReturned)
{
    char mac[] = BRAINFLOW_TEST_BLUETOOTH_VALID_MAC;
    bluetooth_open_device (BRAINFLOW_TEST_BLUETOOTH_NO_BYTES_AVAILABLE_PORT, mac);

    char buffer[BRAINFLOW_TEST_BLUETOOTH_EXPECTED_BYTES]{0};
    int n_bytes = bluetooth_get_data (buffer, BRAINFLOW_TEST_BLUETOOTH_EXPECTED_BYTES, mac);

    EXPECT_EQ (n_bytes, 0);
    EXPECT_THAT (buffer, Contains (BRAINFLOW_TEST_BLUETOOTH_DATA_CHAR).Times (0));

    bluetooth_close_device (mac);
}

TEST (BluetoothTest, GetData_NegativeSize_NoBytesReturned)
{
    char mac[] = BRAINFLOW_TEST_BLUETOOTH_VALID_MAC;
    bluetooth_open_device (BRAINFLOW_TEST_BLUETOOTH_ALL_BYTES_AVAILABLE_PORT, mac);

    char buffer[BRAINFLOW_TEST_BLUETOOTH_EXPECTED_BYTES]{0};
    int n_bytes = bluetooth_get_data (buffer, -1, mac);

    EXPECT_EQ (n_bytes, 0);
    EXPECT_THAT (buffer, Contains (BRAINFLOW_TEST_BLUETOOTH_DATA_CHAR).Times (0));

    bluetooth_close_device (mac);
}

TEST (BluetoothTest, GetData_NullMAC_NoBytesReturned)
{
    char mac[] = BRAINFLOW_TEST_BLUETOOTH_VALID_MAC;
    bluetooth_open_device (BRAINFLOW_TEST_BLUETOOTH_ALL_BYTES_AVAILABLE_PORT, mac);

    char buffer[BRAINFLOW_TEST_BLUETOOTH_EXPECTED_BYTES]{0};
    int n_bytes = bluetooth_get_data (buffer, BRAINFLOW_TEST_BLUETOOTH_EXPECTED_BYTES, nullptr);

    EXPECT_EQ (n_bytes, 0);
    EXPECT_THAT (buffer, Contains (BRAINFLOW_TEST_BLUETOOTH_DATA_CHAR).Times (0));

    bluetooth_close_device (mac);
}

TEST (BluetoothTest, GetData_NullDataBuffer_ZeroReturned)
{
    char mac[] = BRAINFLOW_TEST_BLUETOOTH_VALID_MAC;
    bluetooth_open_device (BRAINFLOW_TEST_BLUETOOTH_ALL_BYTES_AVAILABLE_PORT, mac);

    int n_bytes = bluetooth_get_data (nullptr, BRAINFLOW_TEST_BLUETOOTH_EXPECTED_BYTES, mac);
    EXPECT_EQ (n_bytes, 0);

    bluetooth_close_device (mac);
}

TEST (BluetoothTest, GetData_DeviceNotCreated_NoBytesReturned)
{
    char mac[] = BRAINFLOW_TEST_BLUETOOTH_VALID_MAC;

    char buffer[BRAINFLOW_TEST_BLUETOOTH_EXPECTED_BYTES]{0};
    int n_bytes = bluetooth_get_data (buffer, BRAINFLOW_TEST_BLUETOOTH_EXPECTED_BYTES, mac);

    EXPECT_EQ (n_bytes, 0);
    EXPECT_THAT (buffer, Contains (BRAINFLOW_TEST_BLUETOOTH_DATA_CHAR).Times (0));
}

TEST (BluetoothTest, WriteData_AllBytesCanBeWritten_NumberOfProvidedBytesReturned)
{
    char mac[] = BRAINFLOW_TEST_BLUETOOTH_VALID_MAC;
    bluetooth_open_device (BRAINFLOW_TEST_BLUETOOTH_ALL_BYTES_AVAILABLE_PORT, mac);

    char buffer[8]{'a'};
    int n_bytes = bluetooth_write_data (buffer, 8, mac);

    EXPECT_EQ (n_bytes, 8);

    bluetooth_close_device (mac);
}

TEST (BluetoothTest, WriteData_SomeBytesCanBeWritten_NumberBytesWrittenReturned)
{
    char mac[] = BRAINFLOW_TEST_BLUETOOTH_VALID_MAC;
    bluetooth_open_device (BRAINFLOW_TEST_BLUETOOTH_SOME_BYTES_AVAILABLE_PORT, mac);

    char buffer[8]{'a'};
    int n_bytes = bluetooth_write_data (buffer, 8, mac);

    EXPECT_GT (n_bytes, 0);
    EXPECT_LT (n_bytes, 8);

    bluetooth_close_device (mac);
}

TEST (BluetoothTest, WriteData_NoBytesCanBeWritten_ZeroReturned)
{
    char mac[] = BRAINFLOW_TEST_BLUETOOTH_VALID_MAC;
    bluetooth_open_device (BRAINFLOW_TEST_BLUETOOTH_NO_BYTES_AVAILABLE_PORT, mac);

    char buffer[8]{'a'};
    int n_bytes = bluetooth_write_data (buffer, 8, mac);

    EXPECT_EQ (n_bytes, 0);

    bluetooth_close_device (mac);
}

TEST (BluetoothTest, WriteData_NegativeSize_ZeroReturned)
{
    char mac[] = BRAINFLOW_TEST_BLUETOOTH_VALID_MAC;
    bluetooth_open_device (BRAINFLOW_TEST_BLUETOOTH_ALL_BYTES_AVAILABLE_PORT, mac);

    char buffer[8]{'a'};
    int n_bytes = bluetooth_write_data (buffer, -1, mac);

    EXPECT_EQ (n_bytes, 0);

    bluetooth_close_device (mac);
}

TEST (BluetoothTest, WriteData_NullMAC_ZeroReturned)
{
    char mac[] = BRAINFLOW_TEST_BLUETOOTH_VALID_MAC;
    bluetooth_open_device (BRAINFLOW_TEST_BLUETOOTH_ALL_BYTES_AVAILABLE_PORT, mac);

    char buffer[8]{'a'};
    int n_bytes = bluetooth_write_data (buffer, 8, nullptr);

    EXPECT_EQ (n_bytes, 0);

    bluetooth_close_device (mac);
}

TEST (BluetoothTest, WriteData_NullDataBuffer_ZeroReturned)
{
    char mac[] = BRAINFLOW_TEST_BLUETOOTH_VALID_MAC;
    bluetooth_open_device (BRAINFLOW_TEST_BLUETOOTH_ALL_BYTES_AVAILABLE_PORT, mac);

    int n_bytes = bluetooth_write_data (nullptr, 8, mac);

    EXPECT_EQ (n_bytes, 0);

    bluetooth_close_device (mac);
}

TEST (BluetoothTest, WriteData_DeviceNotCreated_ZeroReturned)
{
    char mac[] = BRAINFLOW_TEST_BLUETOOTH_VALID_MAC;

    char buffer[8]{'a'};
    int n_bytes = bluetooth_write_data (buffer, 8, mac);

    EXPECT_EQ (n_bytes, 0);
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

TEST (BluetoothTest, Discover_DeviceIsNotDiscoverable_DeviceNotCreatedErrorReturned)
{
    int length;
    char mac[32];
    char selector[] = BRAINFLOW_TEST_BLUETOOTH_NOT_DISCOVERABLE_SELECTOR;

    int result = bluetooth_discover_device (selector, mac, &length);

    EXPECT_EQ (result, (int)SocketBluetoothReturnCodes::DEVICE_IS_NOT_CREATED_ERROR);
}

TEST (BluetoothTest, Discover_SelectorNull_DeviceNotCreatedErrorReturned)
{
    int length;
    char mac[32];

    int result = bluetooth_discover_device (nullptr, mac, &length);

    EXPECT_EQ (result, (int)SocketBluetoothReturnCodes::DEVICE_IS_NOT_CREATED_ERROR);
}

TEST (BluetoothTest, Discover_NullMAC_InvalidParameterErrorCodeReturned)
{
    int length;
    char selector[] = BRAINFLOW_TEST_BLUETOOTH_NOT_DISCOVERABLE_SELECTOR;

    int result = bluetooth_discover_device (selector, nullptr, &length);

    EXPECT_EQ (result, (int)SocketBluetoothReturnCodes::DEVICE_IS_NOT_CREATED_ERROR);
}

TEST (BluetoothTest, Discover_NullLengthPointer_InvalidParameterErrorCodeReturned)
{
    char mac[32];
    char selector[] = BRAINFLOW_TEST_BLUETOOTH_NOT_DISCOVERABLE_SELECTOR;

    int result = bluetooth_discover_device (selector, mac, nullptr);

    EXPECT_EQ (result, (int)SocketBluetoothReturnCodes::DEVICE_IS_NOT_CREATED_ERROR);
}