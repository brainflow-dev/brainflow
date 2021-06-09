#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#include "brainflow_constants.h"
#include "cmd_def.h"
#include "muse_bglib_helper.h"
#include "muse_constants.h"
#include "timestamp.h"

#include <iostream>


void ble_evt_connection_status (const struct ble_msg_connection_status_evt_t *msg)
{
    MuseBGLibHelper *helper = MuseBGLibHelper::get_instance ();
    // New connection
    if (msg->flags & connection_connected)
    {
        helper->connection = msg->connection;
        // this method is called from ble_evt_connection_disconnected need to set exit code only
        // when we call this method from open_ble_device
        if (helper->state == (int)DeviceState::INITIAL_CONNECTION)
        {
            helper->exit_code = (int)BrainFlowExitCodes::STATUS_OK;
        }
    }
}

void ble_evt_connection_disconnected (const struct ble_msg_connection_disconnected_evt_t *msg)
{
    // atempt to reconnect
    // changing values here leads to package loss, dont touch it
    MuseBGLibHelper *helper = MuseBGLibHelper::get_instance ();
    ble_cmd_gap_connect_direct (&(helper->connect_addr), gap_address_type_random, 10, 76, 100, 0);
}

// ble_evt_attclient_group_found and ble_evt_attclient_procedure_completed are called after the same
// command(ble_cmd_attclient_read_by_group_type)
void ble_evt_attclient_group_found (const struct ble_msg_attclient_group_found_evt_t *msg)
{
    if (msg->uuid.len == 0)
    {
        return;
    }
    MuseBGLibHelper *helper = MuseBGLibHelper::get_instance ();
    uint16 uuid = (msg->uuid.data[1] << 8) | msg->uuid.data[0];
    if (uuid == MUSE_SERVICE_UUID)
    {
        helper->muse_handle_start = msg->start;
        helper->muse_handle_end = msg->end;
    }
}

void ble_evt_attclient_procedure_completed (
    const struct ble_msg_attclient_procedure_completed_evt_t *msg)
{
    MuseBGLibHelper *helper = MuseBGLibHelper::get_instance ();
    if ((helper->state == (int)DeviceState::WRITE_TO_CLIENT_CHAR) ||
        (helper->state == (int)DeviceState::CONFIG_CALLED))
    {
        if (msg->result == 0)
        {
            helper->exit_code = (int)BrainFlowExitCodes::STATUS_OK;
        }
    }
    if (helper->state == (int)DeviceState::OPEN_CALLED)
    {
        if ((helper->muse_handle_start) && (helper->muse_handle_end))
        {
            ble_cmd_attclient_find_information (msg->connection, helper->muse_handle_start,
                helper->muse_handle_end); // triggers
                                          // ble_evt_attclient_find_information_found
        }
    }
}

void ble_evt_attclient_find_information_found (
    const struct ble_msg_attclient_find_information_found_evt_t *msg)
{
    MuseBGLibHelper *helper = MuseBGLibHelper::get_instance ();
    if (helper->state == (int)DeviceState::OPEN_CALLED)
    {
        if (msg->uuid.len == 2)
        {
            uint16 uuid = (msg->uuid.data[1] << 8) | msg->uuid.data[0];
            std::cout << "uuid " << (int)uuid << " " << (int)msg->chrhandle << std::endl;
            if (uuid == CLIENT_CHARACTERISTIC_UUID)
            {
                helper->ccids.insert (msg->chrhandle);
            }
        }

        if (msg->uuid.len == 16)
        {
            char str[37] = {};
            sprintf (str, "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                msg->uuid.data[15], msg->uuid.data[14], msg->uuid.data[13], msg->uuid.data[12],
                msg->uuid.data[11], msg->uuid.data[10], msg->uuid.data[9], msg->uuid.data[8],
                msg->uuid.data[7], msg->uuid.data[6], msg->uuid.data[5], msg->uuid.data[4],
                msg->uuid.data[3], msg->uuid.data[2], msg->uuid.data[1], msg->uuid.data[0]);
            std::cout << "uuid " << str << " " << (int)msg->chrhandle << std::endl;
            if (strcmp (str, MUSE_GATT_ATTR_STREAM_TOGGLE) == 0)
            {
                std::cout << "control handle " << (int)msg->chrhandle << std::endl;
                helper->characteristics["CONTROL"] = msg->chrhandle;
            }
            if (strcmp (str, MUSE_GATT_ATTR_TP9) == 0)
            {
                helper->characteristics["TP9"] = msg->chrhandle;
            }
            if (strcmp (str, MUSE_GATT_ATTR_AF7) == 0)
            {
                helper->characteristics["AF7"] = msg->chrhandle;
            }
            if (strcmp (str, MUSE_GATT_ATTR_AF8) == 0)
            {
                helper->characteristics["AF8"] = msg->chrhandle;
            }
            if (strcmp (str, MUSE_GATT_ATTR_TP10) == 0)
            {
                helper->characteristics["TP10"] = msg->chrhandle;
            }
        }
        if ((helper->characteristics.size () == 5) &&
            (helper->state == (int)DeviceState::OPEN_CALLED))
        {
            std::cout << "all found" << helper->ccids.size () << std::endl;
            helper->exit_code = (int)BrainFlowExitCodes::STATUS_OK;
        }
    }
}

void ble_evt_attclient_attribute_value (const struct ble_msg_attclient_attribute_value_evt_t *msg)
{
    // todo
    std::cout << "triggered " << (int)msg->value.len << " " << (int)msg->atthandle << std::endl;
}

void ble_evt_gap_scan_response (const struct ble_msg_gap_scan_response_evt_t *msg)
{
    char name[512];
    bool name_found_in_response = false;
    for (int i = 0; i < msg->data.len;)
    {
        int8 len = msg->data.data[i++];
        if (!len)
        {
            continue;
        }
        if (i + len > msg->data.len)
        {
            break; // not enough data
        }
        uint8 type = msg->data.data[i++];
        if (type == 0x09) // no idea what is 0x09
        {
            name_found_in_response = true;
            memcpy (name, msg->data.data + i, len - 1);
            name[len - 1] = '\0';
        }

        i += len - 1;
    }

    if (name_found_in_response)
    {
        if (strstr (name, "Muse") != NULL)
        {
            MuseBGLibHelper *helper = MuseBGLibHelper::get_instance ();
            memcpy ((void *)helper->connect_addr.addr, msg->sender.addr, sizeof (bd_addr));
            helper->exit_code = (int)BrainFlowExitCodes::STATUS_OK;
        }
    }
}