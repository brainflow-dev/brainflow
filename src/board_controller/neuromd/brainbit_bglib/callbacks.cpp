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
#include "ticket_lock.h"
#include "timestamp.h"
#include "uart.h"


/*
export const DEVICE_INFORMATION_SERVICE = 0x180A;
export const BRAINBIT_SERVICE = '6e400001-b534-f393-68a9-e50e24dcca9e';
export const BLE_UUID_NSS2_STATUS_CHAR = '6e400002-b534-f393-68a9-e50e24dcca9e';
export const BLE_UUID_NSS2_COMMAND_CHAR = '6e400003-b534-f393-68a9-e50e24dcca9e';
export const BLE_UUID_NSS2_SIGNAL_CHAR = '6e400004-b534-f393-68a9-e50e24dcca9e';
*/

#define CLIENT_CHARACTERISTIC_UUID 0x2902

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
    extern std::set<uint16> ccids;
    extern volatile State state;
    extern TicketLock lock;
    extern volatile double battery_level;

    extern std::deque<std::array<double, BRAINBIT_BLED_DATA_SIZE>> data_queue;

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
            BrainBitBLEDLib::exit_code = (int)BrainFlowExitCodes::STATUS_OK;
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
            BrainBitBLEDLib::exit_code = (int)BrainFlowExitCodes::STATUS_OK;
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
            BrainBitBLEDLib::exit_code = (int)BrainFlowExitCodes::STATUS_OK;
        }
    }
}

void ble_evt_attclient_find_information_found (
    const struct ble_msg_attclient_find_information_found_evt_t *msg)
{
    if (BrainBitBLEDLib::state == BrainBitBLEDLib::State::OPEN_CALLED)
    {
        if (msg->uuid.len == 2)
        {
            uint16 uuid = (msg->uuid.data[1] << 8) | msg->uuid.data[0];
            if (uuid == CLIENT_CHARACTERISTIC_UUID)
            {
                BrainBitBLEDLib::ccids.insert (msg->chrhandle);
            }
        }

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
            (BrainBitBLEDLib::brainbit_handle_status) && (BrainBitBLEDLib::ccids.size () > 1) &&
            (BrainBitBLEDLib::state == BrainBitBLEDLib::State::OPEN_CALLED))
        {
            BrainBitBLEDLib::exit_code = (int)BrainFlowExitCodes::STATUS_OK;
        }
    }
}

void ble_evt_attclient_attribute_value (const struct ble_msg_attclient_attribute_value_evt_t *msg)
{
    if (msg->value.len == 20)
    {
        unsigned char values[20] = {0};
        memcpy (values, msg->value.data, msg->value.len * sizeof (unsigned char));
        double timestamp = get_timestamp ();
        std::array<double, BRAINBIT_BLED_DATA_SIZE> package;

        double multiplier = 2.4 * 1000000 / (0xFFFFF * 6);
        package[0] = (double)((msg->value.data[0] << 3) | (msg->value.data[1] >> 5));

        double eeg01 = ((msg->value.data[1] & 0x0F) << 28) | (msg->value.data[2] << 20) |
            (msg->value.data[3] << 12) | (msg->value.data[4] << 4);
        eeg01 /= 2048;
        double eeg02 = ((msg->value.data[4] & 0x7F) << 25) | (msg->value.data[5] << 17) |
            (msg->value.data[6] << 9) | (msg->value.data[7] << 1);
        eeg02 /= 2048;
        double eeg03 = ((msg->value.data[6] & 0x03) << 30) | (msg->value.data[7] << 22) |
            (msg->value.data[8] << 14) | (msg->value.data[9] << 6);
        eeg03 /= 2048;
        double eeg04 = ((msg->value.data[9] & 0x1F) << 27) | (msg->value.data[10] << 19) |
            (msg->value.data[11] << 11);
        eeg04 /= 2048;

        double eeg11 = (msg->value.data[12] << 24) | (msg->value.data[13] << 16);
        eeg11 = eeg11 / 65536 + eeg01;
        double eeg12 = (msg->value.data[14] << 24) | (msg->value.data[15] << 16);
        eeg12 = eeg12 / 65536 + eeg02;
        double eeg13 = (msg->value.data[16] << 24) | (msg->value.data[17] << 16);
        eeg13 = eeg13 / 65536 + eeg03;
        double eeg14 = (msg->value.data[18] << 24) | (msg->value.data[19] << 16);
        eeg14 = eeg14 / 65536 + eeg04;

        // order of packages: 0 - O1, 1 - T3, 2- T4, 3 - O2, need to  swap them to match BrainBit
        // board (t3, t4, o1, o2)
        package[1] = eeg02 * multiplier;
        package[2] = eeg03 * multiplier;
        package[3] = eeg01 * multiplier;
        package[4] = eeg04 * multiplier;
        package[5] = BrainBitBLEDLib::battery_level;
        package[6] = get_timestamp ();

        BrainBitBLEDLib::lock.lock ();
        BrainBitBLEDLib::data_queue.push_back (package);
        BrainBitBLEDLib::lock.unlock ();

        package[1] = eeg12 * multiplier;
        package[2] = eeg13 * multiplier;
        package[3] = eeg11 * multiplier;
        package[4] = eeg14 * multiplier;
        package[5] = BrainBitBLEDLib::battery_level;
        package[6] = get_timestamp ();


        BrainBitBLEDLib::lock.lock ();
        BrainBitBLEDLib::data_queue.push_back (package);
        BrainBitBLEDLib::lock.unlock ();
    }
    if (msg->value.len == 4)
    {
        BrainBitBLEDLib::battery_level = (double)msg->value.data[2];
    }
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
            BrainBitBLEDLib::exit_code = (int)BrainFlowExitCodes::STATUS_OK;
        }
    }
}