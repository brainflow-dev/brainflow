#include <array>
#include <chrono>
#include <ctime>
#include <ctype.h>
#include <deque>
#include <math.h>
#include <set>
#include <stdlib.h>
#include <string.h>

#include "brainflow_constants.h"
#include "cmd_def.h"
#include "helpers.h"
#include "muse_types.h"
#include "ticket_lock.h"
#include "timestamp.h"
#include "uart.h"


#include "iostream"


#define CLIENT_CHARACTERISTIC_UUID 0x2902

namespace MuseBLEDLib
{
    extern volatile int exit_code;
    extern volatile bd_addr connect_addr;
    extern volatile uint8 connection;
    extern volatile uint16 muse_handle_start;
    extern volatile uint16 muse_handle_end;
    // recv and send chars handles
    extern volatile uint16 muse_handle_recv;
    extern volatile uint16 muse_handle_send;
    extern volatile uint16 muse_handle_status;
    extern std::set<uint16> ccids;
    extern volatile State state;
    extern TicketLock lock;
    extern volatile double battery_level;

    extern std::deque<std::array<double, MUSE_S_DATA_SIZE>> data_queue;

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
        MuseBLEDLib::connection = msg->connection;
        // this method is called from ble_evt_connection_disconnected need to set exit code only
        // when we call this method from open_ble_device
        if (MuseBLEDLib::state == MuseBLEDLib::State::INITIAL_CONNECTION)
        {
            MuseBLEDLib::exit_code = (int)BrainFlowExitCodes::STATUS_OK;
        }
    }
}

void ble_evt_connection_disconnected (const struct ble_msg_connection_disconnected_evt_t *msg)
{
    // atempt to reconnect
    // changing values here leads to package loss, dont touch it
    ble_cmd_gap_connect_direct (
        &MuseBLEDLib::connect_addr, gap_address_type_random, 10, 76, 100, 0);
}

// ble_evt_attclient_group_found and ble_evt_attclient_procedure_completed are called after the same
// command(ble_cmd_attclient_read_by_group_type)
void ble_evt_attclient_group_found (const struct ble_msg_attclient_group_found_evt_t *msg)
{
    uint16 uuid = (msg->uuid.data[1] << 8) | msg->uuid.data[0];
    std::cout << "asd" << (int)msg->uuid.len << " " << int (uuid) << std::endl;
    if (msg->uuid.len == 0)
    {
        return;
    }
    bool is_valid = true;
    if (msg->uuid.len == 16)
    {
        for (int i = 0; i < 16; i++)
        {
            if (msg->uuid.data[i] != MuseBLEDLib::service_uuid_bytes[i])
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
        MuseBLEDLib::muse_handle_start = msg->start;
        MuseBLEDLib::muse_handle_end = msg->end;
    }
}

void ble_evt_attclient_procedure_completed (
    const struct ble_msg_attclient_procedure_completed_evt_t *msg)
{
    std::cout << "zxc" << std::endl;
    if (MuseBLEDLib::state == MuseBLEDLib::State::WRITE_TO_CLIENT_CHAR)
    {
        if (msg->result == 0)
        {
            MuseBLEDLib::exit_code = (int)BrainFlowExitCodes::STATUS_OK;
        }
    }
    if (MuseBLEDLib::state == MuseBLEDLib::State::OPEN_CALLED)
    {
        if ((MuseBLEDLib::muse_handle_start) && (MuseBLEDLib::muse_handle_end))
        {
            ble_cmd_attclient_find_information (msg->connection, MuseBLEDLib::muse_handle_start,
                MuseBLEDLib::muse_handle_end); // triggers
                                               // ble_evt_attclient_find_information_found
        }
    }
    else if (MuseBLEDLib::state == MuseBLEDLib::State::CONFIG_CALLED)
    {
        if (msg->result == 0)
        {
            MuseBLEDLib::exit_code = (int)BrainFlowExitCodes::STATUS_OK;
        }
    }
}

void ble_evt_attclient_find_information_found (
    const struct ble_msg_attclient_find_information_found_evt_t *msg)
{
    std::cout << "len " << (int)msg->uuid.len << std::endl;
    if (MuseBLEDLib::state == MuseBLEDLib::State::OPEN_CALLED)
    {
        if (msg->uuid.len == 2)
        {
            uint16 uuid = (msg->uuid.data[1] << 8) | msg->uuid.data[0];
            std::cout << "uuid" << (int)uuid << std::endl;
            if (uuid == CLIENT_CHARACTERISTIC_UUID)
            {
                MuseBLEDLib::ccids.insert (msg->chrhandle);
            }
        }

        if (msg->uuid.len == 16)
        {
            char str[37] = {};
            sprintf (str, "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                msg->uuid.data[0], msg->uuid.data[1], msg->uuid.data[2], msg->uuid.data[3],
                msg->uuid.data[4], msg->uuid.data[5], msg->uuid.data[6], msg->uuid.data[7],
                msg->uuid.data[8], msg->uuid.data[9], msg->uuid.data[10], msg->uuid.data[11],
                msg->uuid.data[12], msg->uuid.data[13], msg->uuid.data[14], msg->uuid.data[15]);
            std::cout << "uuid" << str << std::endl;
            bool is_send = true;
            bool is_recv = true;
            bool is_status = true;
            for (int i = 0; i < 16; i++)
            {
                if (msg->uuid.data[i] != MuseBLEDLib::command_char_uuid_bytes[i])
                {
                    is_send = false;
                }
            }
            if (is_send)
            {
                MuseBLEDLib::muse_handle_send = msg->chrhandle;
            }
        }
        if ((MuseBLEDLib::muse_handle_send) &&
            (MuseBLEDLib::state == MuseBLEDLib::State::OPEN_CALLED))
        {
            MuseBLEDLib::exit_code = (int)BrainFlowExitCodes::STATUS_OK;
        }
    }
}

void ble_evt_attclient_attribute_value (const struct ble_msg_attclient_attribute_value_evt_t *msg)
{
    std::cout << "qwe" << std::endl;
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

    std::cout << name << std::endl;

    if (name_found_in_response)
    {
        if (strstr (name, "Muse") != NULL)
        {
            memcpy ((void *)MuseBLEDLib::connect_addr.addr, msg->sender.addr, sizeof (bd_addr));
            MuseBLEDLib::exit_code = (int)BrainFlowExitCodes::STATUS_OK;
        }
    }
}