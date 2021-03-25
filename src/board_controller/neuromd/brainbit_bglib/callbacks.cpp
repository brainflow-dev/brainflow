#include <ctype.h>
#include <deque>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "brainbit_types.h"
#include "cmd_def.h"
#include "helpers.h"
#include "ticket_lock.h"
#include "timestamp.h"
#include "uart.h"
#include <chrono>
#include <ctime>

#include <iostream>

/*
export const DEVICE_INFORMATION_SERVICE = 0x180A;
export const BRAINBIT_SERVICE = '6e400001-b534-f393-68a9-e50e24dcca9e';
export const BLE_UUID_NSS2_STATUS_CHAR = '6e400002-b534-f393-68a9-e50e24dcca9e';
export const BLE_UUID_NSS2_COMMAND_CHAR = '6e400003-b534-f393-68a9-e50e24dcca9e';
export const BLE_UUID_NSS2_SIGNAL_CHAR = '6e400004-b534-f393-68a9-e50e24dcca9e';
*/

namespace BrainBitBLEDLib
{
    extern volatile int exit_code;
    extern volatile bd_addr connect_addr;
    extern volatile uint8 connection;
    extern volatile uint16 brainbit_handle_start;
    extern volatile uint16 brainbit_handle_end;
    // recv and send chars handles
    extern volatile uint16 brainbit_handle_recv;
    extern volatile uint16 brainbit_handle_send;
    extern volatile uint16 brainbit_handle_status;
    extern volatile State state;
    extern TicketLock lock;

    extern std::deque<struct BrainBitBLEDLib::BrainBitData> data_queue;

    // 6e400001-b534-f393-68a9-e50e24dcca9e
    const int service_uuid_bytes[16] = {
        158, 202, 220, 36, 14, 229, 169, 104, 147, 243, 52, 181, 1, 0, 64, 110};
    // 6e400002-b534-f393-68a9-e50e24dcca9e
    const int status_char_uuid_bytes[16] = {
        158, 202, 220, 36, 14, 229, 169, 104, 147, 243, 52, 181, 2, 0, 64, 110};
    // 6e400003-b534-f393-68a9-e50e24dcca9e
    const int command_char_uuid_bytes[16] = {
        158, 202, 220, 36, 14, 229, 169, 104, 147, 243, 52, 181, 3, 0, 64, 110};
    // 6e400004-b534-f393-68a9-e50e24dcca9e
    const int signal_char_uuid_bytes[16] = {
        158, 202, 220, 36, 14, 229, 169, 104, 147, 243, 52, 181, 4, 0, 64, 110};
}

void ble_evt_connection_status (const struct ble_msg_connection_status_evt_t *msg)
{
    // New connection
    if (msg->flags & connection_connected)
    {
        BrainBitBLEDLib::connection = msg->connection;
        // this method is called from ble_evt_connection_disconnected need to set exit code only
        // when we call this method from open_ble_device
        if (BrainBitBLEDLib::state == BrainBitBLEDLib::State::INITIAL_CONNECTION)
        {
            BrainBitBLEDLib::exit_code = (int)BrainBitBLEDLib::STATUS_OK;
        }
    }
}

void ble_evt_connection_disconnected (const struct ble_msg_connection_disconnected_evt_t *msg)
{
    // atempt to reconnect
    // changing values here leads to package loss, dont touch it
    ble_cmd_gap_connect_direct (
        &BrainBitBLEDLib::connect_addr, gap_address_type_random, 10, 76, 100, 0);
}

// ble_evt_attclient_group_found and ble_evt_attclient_procedure_completed are called after the same
// command(ble_cmd_attclient_read_by_group_type)
void ble_evt_attclient_group_found (const struct ble_msg_attclient_group_found_evt_t *msg)
{
    if (msg->uuid.len == 0)
    {
        return;
    }
    bool is_valid = true;
    if (msg->uuid.len == 16)
    {
        for (int i = 0; i < 16; i++)
        {
            if (msg->uuid.data[i] != BrainBitBLEDLib::service_uuid_bytes[i])
            {
                is_valid = false;
            }
        }
    }
    else
    {
        is_valid = false;
    }
    if (is_valid)
    {
        BrainBitBLEDLib::brainbit_handle_start = msg->start;
        BrainBitBLEDLib::brainbit_handle_end = msg->end;
    }
}

void ble_evt_attclient_procedure_completed (
    const struct ble_msg_attclient_procedure_completed_evt_t *msg)
{
    if (BrainBitBLEDLib::state == BrainBitBLEDLib::State::WRITE_TO_CLIENT_CHAR)
    {
        if (msg->result == 0)
        {
            BrainBitBLEDLib::exit_code = (int)BrainBitBLEDLib::STATUS_OK;
        }
    }
    if (BrainBitBLEDLib::state == BrainBitBLEDLib::State::OPEN_CALLED)
    {
        if ((BrainBitBLEDLib::brainbit_handle_start) && (BrainBitBLEDLib::brainbit_handle_end))
        {
            ble_cmd_attclient_find_information (msg->connection,
                BrainBitBLEDLib::brainbit_handle_start,
                BrainBitBLEDLib::brainbit_handle_end); // triggers
                                                       // ble_evt_attclient_find_information_found
        }
    }
    else if (BrainBitBLEDLib::state == BrainBitBLEDLib::State::CONFIG_CALLED)
    {
        if (msg->result == 0)
        {
            BrainBitBLEDLib::exit_code = (int)BrainBitBLEDLib::STATUS_OK;
        }
    }
}

void ble_evt_attclient_find_information_found (
    const struct ble_msg_attclient_find_information_found_evt_t *msg)
{
    if (BrainBitBLEDLib::state == BrainBitBLEDLib::State::OPEN_CALLED)
    {
        for (int i = 0; i < msg->uuid.len; i++)
        {
            std::cout << (int)msg->uuid.data[i] << " ";
        }
        if (msg->uuid.len == 2)
        {
            uint16 uuid = (msg->uuid.data[1] << 8) | msg->uuid.data[0];
            std::cout << (int)uuid << " uuid " << std::endl;
        }

        std::cout << std::endl;
        if (msg->uuid.len == 16)
        {
            bool is_send = true;
            bool is_recv = true;
            bool is_status = true;
            for (int i = 0; i < 16; i++)
            {
                if (msg->uuid.data[i] != BrainBitBLEDLib::command_char_uuid_bytes[i])
                {
                    is_send = false;
                }
                if (msg->uuid.data[i] != BrainBitBLEDLib::signal_char_uuid_bytes[i])
                {
                    is_recv = false;
                }
                if (msg->uuid.data[i] != BrainBitBLEDLib::status_char_uuid_bytes[i])
                {
                    is_status = false;
                }
            }
            if (is_recv)
            {
                BrainBitBLEDLib::brainbit_handle_recv = msg->chrhandle;
            }
            if (is_send)
            {
                BrainBitBLEDLib::brainbit_handle_send = msg->chrhandle;
            }
            if (is_status)
            {
                BrainBitBLEDLib::brainbit_handle_status = msg->chrhandle;
            }
        }
        if ((BrainBitBLEDLib::brainbit_handle_send) && (BrainBitBLEDLib::brainbit_handle_recv) &&
            (BrainBitBLEDLib::brainbit_handle_status) &&
            (BrainBitBLEDLib::state == BrainBitBLEDLib::State::OPEN_CALLED))
        {
            BrainBitBLEDLib::exit_code = (int)BrainBitBLEDLib::STATUS_OK;
        }
    }
}

void ble_evt_attclient_attribute_value (const struct ble_msg_attclient_attribute_value_evt_t *msg)
{
    unsigned char values[20] = {0};
    memcpy (values, msg->value.data, msg->value.len * sizeof (unsigned char));
    double timestamp = get_timestamp ();
    double package[BrainBitBLEDLib::BrainBitData::SIZE] = {0.0};

    // TODO parse message and write data to package
    std::cout << "called" << std::endl;

    struct BrainBitBLEDLib::BrainBitData data (package);

    BrainBitBLEDLib::lock.lock ();
    BrainBitBLEDLib::data_queue.push_back (data);
    BrainBitBLEDLib::lock.unlock ();
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
        if (strstr (name, "BrainBit") != NULL)
        {
            memcpy ((void *)BrainBitBLEDLib::connect_addr.addr, msg->sender.addr, sizeof (bd_addr));
            BrainBitBLEDLib::exit_code = (int)BrainBitBLEDLib::STATUS_OK;
        }
    }
}