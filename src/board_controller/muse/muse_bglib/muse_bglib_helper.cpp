#include <string.h>
#include <string>

#include "brainflow_constants.h"
#include "muse_bglib_helper.h"
#include "muse_constants.h"
#include "muse_types.h"
#include "uart.h"

#include <iostream>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#define UART_TIMEOUT 1000

MuseBGLibHelper *MuseBGLibHelper::instance = NULL;


void output (uint8 len1, uint8 *data1, uint16 len2, uint8 *data2)
{
    if (uart_tx (len1, data1) || uart_tx (len2, data2))
    {
        MuseBGLibHelper::get_instance ()->exit_code =
            (int)BrainFlowExitCodes::UNABLE_TO_OPEN_PORT_ERROR;
    }
}

MuseBGLibHelper *MuseBGLibHelper::get_instance ()
{
    if (instance == NULL)
    {
        instance = new MuseBGLibHelper ();
        instance->reset ();
    }
    return instance;
}

void MuseBGLibHelper::reset ()
{
    stop_stream ();
    exit_code = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
    connection = -1;
    muse_handle_start = 0;
    muse_handle_end = 0;
    state = (int)DeviceState::NONE;
    should_stop_stream = true;
    initialized = false;
    board_id = (int)BoardIds::SYNTHETIC_BOARD;
    ccids.clear ();
    characteristics.clear ();
}

void MuseBGLibHelper::thread_worker ()
{
    while (!should_stop_stream)
    {
        read_message (UART_TIMEOUT);
    }
}

int MuseBGLibHelper::read_message (int timeout_ms)
{
    unsigned char *data = NULL;
    struct ble_header hdr;
    int r;

    r = uart_rx (sizeof (hdr), (unsigned char *)&hdr, timeout_ms);
    if (!r)
    {
        return -1; // timeout
    }
    else if (r < 0)
    {
        exit_code = (int)BrainFlowExitCodes::UNABLE_TO_OPEN_PORT_ERROR;
        return 1; // fails to read
    }
    if (hdr.lolen)
    {
        data = new unsigned char[hdr.lolen];
        r = uart_rx (hdr.lolen, data, UART_TIMEOUT);
        if (r <= 0)
        {
            exit_code = (int)BrainFlowExitCodes::UNABLE_TO_OPEN_PORT_ERROR;
            delete[] data;
            return 1; // fails to read
        }
    }

    const struct ble_msg *msg = ble_get_msg_hdr (hdr);
    if (msg)
    {
        msg->handler (data);
    }
    delete[] data;
    return 0;
}

int MuseBGLibHelper::open_ble_dev ()
{
    exit_code = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
    // send command to connect
    state = (int)DeviceState::INITIAL_CONNECTION;
    ble_cmd_gap_connect_direct (&connect_addr, gap_address_type_public, 10, 76, 100, 0);
    int res = wait_for_callback ();
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }
    state = (int)DeviceState::OPEN_CALLED;
    exit_code = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
    uint8 primary_service_uuid[] = {0x00, 0x28};

    ble_cmd_attclient_read_by_group_type (
        connection, FIRST_HANDLE, LAST_HANDLE, 2, primary_service_uuid);

    res = wait_for_callback ();
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }

    // from siliconlabs forum - write 0x00001 to enable notifications
    // copypasted in start_stream method but lets keep it in 2 places
    uint8 configuration[] = {0x01, 0x00};
    for (uint16 ccid : ccids)
    {
        state = (int)DeviceState::WRITE_TO_CLIENT_CHAR;
        exit_code = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
        ble_cmd_attclient_attribute_write (connection, ccid, 2, &configuration);
        ble_cmd_attclient_execute_write (connection, 1);
        wait_for_callback ();
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int MuseBGLibHelper::wait_for_callback ()
{
    auto start_time = std::chrono::high_resolution_clock::now ();
    int run_time = 0;
    while ((run_time < input_params.timeout) &&
        (exit_code == (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR))
    {
        if (read_message (UART_TIMEOUT) > 0)
        {
            break;
        }
        auto end_time = std::chrono::high_resolution_clock::now ();
        run_time =
            (int)std::chrono::duration_cast<std::chrono::seconds> (end_time - start_time).count ();
    }
    return exit_code;
}

int MuseBGLibHelper::reset_ble_dev ()
{
    // Reset dongle to get it into known state
    ble_cmd_system_reset (0);
    uart_close ();
    int i;
    for (i = 0; i < 5; i++)
    {
#ifdef _WIN32
        Sleep (500);
#else
        usleep (500000);
#endif
        if (!uart_open (input_params.serial_port.c_str ()))
        {
            break;
        }
    }
    if (i == 5)
    {
        return (int)BrainFlowExitCodes::UNABLE_TO_OPEN_PORT_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int MuseBGLibHelper::initialize (void *param)
{
    if (!initialized)
    {
        if (param == NULL)
        {
            return (int)BrainFlowExitCodes::UNABLE_TO_OPEN_PORT_ERROR;
        }

        std::pair<int, struct BrainFlowInputParams> *info =
            (std::pair<int, struct BrainFlowInputParams> *)param;
        board_id = info->first;
        input_params = info->second;
        bglib_output = output;
        exit_code = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
        initialized = true;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int MuseBGLibHelper::open_device ()
{
    if (!initialized)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    if (uart_open (input_params.serial_port.c_str ()))
    {
        return (int)BrainFlowExitCodes::UNABLE_TO_OPEN_PORT_ERROR;
    }
    int res = reset_ble_dev ();
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }
    exit_code = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
    state = (int)DeviceState::OPEN_CALLED;
    ble_cmd_gap_discover (gap_discover_observation);
    res = wait_for_callback ();
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }
    ble_cmd_gap_end_procedure ();
    return open_ble_dev ();
}

int MuseBGLibHelper::stop_stream ()
{
    if (!initialized)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    if (!should_stop_stream)
    {
        should_stop_stream = true;
        read_characteristic_thread.join ();
    }
    int res = (int)BrainFlowExitCodes::STATUS_OK;
    // for sanity check
    for (int i = 0; i < 5; i++)
    {
        const char *stop_cmd = "h";
        res = config_device ((void *)stop_cmd);
    }
    return res;
}

int MuseBGLibHelper::start_stream ()
{
    // from silicanlabs forum - write 0x00001 to enable notifications
    MuseBGLibHelper *helper = MuseBGLibHelper::get_instance ();
    if (!initialized)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    uint8 configuration[] = {0x01, 0x00};
    for (uint16 ccid : ccids)
    {
        state = (int)DeviceState::WRITE_TO_CLIENT_CHAR;
        exit_code = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
        ble_cmd_attclient_attribute_write (connection, ccid, 2, &configuration);
        ble_cmd_attclient_execute_write (connection, 1);
        int res = wait_for_callback ();
        if (res != (int)BrainFlowExitCodes::STATUS_OK)
        {
            return res;
        }
    }
    int res = config_device ((void *)"v1");
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }
    res = config_device ((void *)"p63");
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }
    res = config_device ((void *)"d");
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        should_stop_stream = false;
        read_characteristic_thread = std::thread ([this] { this->thread_worker (); });
    }

    return res;
}

int MuseBGLibHelper::close_device ()
{
    if (!initialized)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    state = (int)DeviceState::CLOSE_CALLED;
    stop_stream ();
    uart_close ();
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int MuseBGLibHelper::get_data (void *param)
{
    if (!initialized)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    state = (int)DeviceState::GET_DATA_CALLED;
    int res = (int)BrainFlowExitCodes::EMPTY_BUFFER_ERROR;
    return res;
}

int MuseBGLibHelper::release ()
{
    if (initialized)
    {
        close_device ();
        reset ();
    }

    return (int)BrainFlowExitCodes::STATUS_OK;
}

int MuseBGLibHelper::config_device (void *param)
{
    MuseBGLibHelper *helper = MuseBGLibHelper::get_instance ();
    if (!initialized)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    unsigned char *config = (unsigned char *)param;
    uint8 command[16];
    int len = (int)strlen ((char *)config);
    command[0] = (uint8)len + 1;
    for (int i = 0; i < len; i++)
    {
        command[i + 1] = int (config[i]);
    }
    command[len + 1] = 10;
    for (int i = 0; i < len + 2; i++)
    {
        std::cout << (int)command[i] << " ";
    }
    std::cout << std::endl;

    if (characteristics.find ("CONTROL") == characteristics.end ())
    {
        return (int)BrainFlowExitCodes::BOARD_WRITE_ERROR;
    }
    ble_cmd_attclient_write_command (connection, characteristics["CONTROL"], len + 2, command);
    return (int)BrainFlowExitCodes::STATUS_OK;
}
