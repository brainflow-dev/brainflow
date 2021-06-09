#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#include "brainflow_constants.h"
#include "cmd_def.h"
#include "muse_bglib_helper.h"
#include "muse_constants.h"
#include "muse_functions.h"
#include "muse_types.h"
#include "uart.h"

#include <iostream>
// read Bluetooth_Smart_Software_v1.3.1_API_Reference.pdf to understand this code


void output (uint8 len1, uint8 *data1, uint16 len2, uint8 *data2)
{
    if (uart_tx (len1, data1) || uart_tx (len2, data2))
    {
        MuseBGLibHelper::get_instance ()->exit_code =
            (int)BrainFlowExitCodes::UNABLE_TO_OPEN_PORT_ERROR;
    }
}

int initialize (void *param)
{
    MuseBGLibHelper *helper = MuseBGLibHelper::get_instance ();
    if (!helper->initialized)
    {
        if (param == NULL)
        {
            return (int)BrainFlowExitCodes::UNABLE_TO_OPEN_PORT_ERROR;
        }

        struct MuseInputData *input = (struct MuseInputData *)param;
        strcpy (helper->uart_port, input->uart_port);
        helper->timeout = input->timeout;
        bglib_output = output;
        helper->exit_code = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
        helper->initialized = true;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int open_device (void *param)
{
    MuseBGLibHelper *helper = MuseBGLibHelper::get_instance ();
    if (!helper->initialized)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    if (uart_open (helper->uart_port))
    {
        return (int)BrainFlowExitCodes::UNABLE_TO_OPEN_PORT_ERROR;
    }
    int res = reset_ble_dev ();
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }
    helper->exit_code = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
    helper->state = (int)DeviceState::OPEN_CALLED;
    ble_cmd_gap_discover (gap_discover_observation);
    res = wait_for_callback (helper->timeout);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }
    ble_cmd_gap_end_procedure ();
    return open_ble_dev ();
}

int open_device_mac_addr (void *param)
{
    MuseBGLibHelper *helper = MuseBGLibHelper::get_instance ();
    if (!helper->initialized)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    if (uart_open (helper->uart_port))
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    int res = reset_ble_dev ();
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }
    helper->exit_code = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
    helper->state = (int)DeviceState::OPEN_CALLED;
    char *mac_addr = (char *)param;
    // convert string mac addr to bd_addr struct
    for (unsigned int i = 0; i < strlen (mac_addr); i++)
    {
        mac_addr[i] = tolower (mac_addr[i]);
    }
    short unsigned int addr[6];
    if (sscanf (mac_addr, "%02hx:%02hx:%02hx:%02hx:%02hx:%02hx", &addr[5], &addr[4], &addr[3],
            &addr[2], &addr[1], &addr[0]) == 6)
    {
        for (int i = 0; i < 6; i++)
        {
            helper->connect_addr.addr[i] = (uint8)addr[i];
        }
    }
    else
    {
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    return open_ble_dev ();
}

int stop_stream (void *param)
{
    MuseBGLibHelper *helper = MuseBGLibHelper::get_instance ();
    if (!helper->initialized)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    helper->stop_stream ();
    int res = (int)BrainFlowExitCodes::STATUS_OK;
    // for sanity check
    for (int i = 0; i < 5; i++)
    {
        const char *stop_cmd = "h";
        res = config_device ((void *)stop_cmd);
    }
    return res;
}

int start_stream (void *param)
{
    // from silicanlabs forum - write 0x00001 to enable notifications
    MuseBGLibHelper *helper = MuseBGLibHelper::get_instance ();
    if (!helper->initialized)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    uint8 configuration[] = {0x01, 0x00};
    for (uint16 ccid : helper->ccids)
    {
        helper->state = (int)DeviceState::WRITE_TO_CLIENT_CHAR;
        helper->exit_code = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
        ble_cmd_attclient_attribute_write (helper->connection, ccid, 2, &configuration);
        ble_cmd_attclient_execute_write (helper->connection, 1);
        int res = wait_for_callback (helper->timeout);
        if (res != (int)BrainFlowExitCodes::STATUS_OK)
        {
            return res;
        }
    }
    int res = config_device ((void *)"v1");
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        std::cout << "failed to write 1" << std::endl;
        return res;
    }
    res = config_device ((void *)"p63");
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        std::cout << "failed to write 2" << std::endl;
        return res;
    }
    res = config_device ((void *)"d");
    helper->start_stream ();

    return res;
}

int close_device (void *param)
{
    MuseBGLibHelper *helper = MuseBGLibHelper::get_instance ();
    if (!helper->initialized)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    helper->state = (int)DeviceState::CLOSE_CALLED;
    helper->stop_stream ();
    uart_close ();
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int get_data (void *param)
{
    MuseBGLibHelper *helper = MuseBGLibHelper::get_instance ();
    if (!helper->initialized)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    helper->state = (int)DeviceState::GET_DATA_CALLED;
    int res = (int)BrainFlowExitCodes::EMPTY_BUFFER_ERROR;
    return res;
}

int release (void *param)
{
    MuseBGLibHelper *helper = MuseBGLibHelper::get_instance ();
    if (helper->initialized)
    {
        close_device (NULL);
        helper->reset ();
    }

    return (int)BrainFlowExitCodes::STATUS_OK;
}

int config_device (void *param)
{
    MuseBGLibHelper *helper = MuseBGLibHelper::get_instance ();
    if (!helper->initialized)
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

    if (helper->characteristics.find ("CONTROL") == helper->characteristics.end ())
    {
        return (int)BrainFlowExitCodes::BOARD_WRITE_ERROR;
    }
    ble_cmd_attclient_write_command (
        helper->connection, helper->characteristics["CONTROL"], len + 2, command);
    return (int)BrainFlowExitCodes::STATUS_OK;
}
