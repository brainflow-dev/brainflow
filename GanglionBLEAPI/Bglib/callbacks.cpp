#include <ctype.h>
#include <math.h>
#include <queue>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "GanglionNativeInterface.h"
#include "cmd_def.h"
#include "helpers.h"
#include "timestamp.h"
#include "uart.h"

#define GANGLION_SERVICE_UUID 0xfe84
#define CLIENT_CHARACTERISTIC_UUID 0x2902

extern volatile int exit_code;
extern volatile bd_addr connect_addr;
extern volatile uint8 connection;
extern volatile uint16 ganglion_handle_start;
extern volatile uint16 ganglion_handle_end;
// recv and send chars handles
extern volatile uint16 ganglion_handle_recv;
extern volatile uint16 ganglion_handle_send;
extern volatile uint16 client_char_handle;
extern volatile State state;

extern std::queue<struct GanglionLibNative::GanglionDataNative> data_queue;

// uuid - 2d30c083-f39f-4ce6-923f-3484ea480596
const int send_char_uuid_bytes[16] = {
    150, 5, 72, 234, 132, 52, 63, 146, 230, 76, 159, 243, 131, 192, 48, 45};
// uuid - 2d30c082-f39f-4ce6-923f-3484ea480596
const int recv_char_uuid_bytes[16] = {
    150, 5, 72, 234, 132, 52, 63, 146, 230, 76, 159, 243, 130, 192, 48, 45};

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
        if (strcasestr (name, "ganglion") != NULL)
        {
            memcpy ((void *)connect_addr.addr, msg->sender.addr, sizeof (bd_addr));
            exit_code = (int)GanglionLibNative::STATUS_OK;
        }
    }
}

void ble_evt_connection_status (const struct ble_msg_connection_status_evt_t *msg)
{
    // New connection
    if (msg->flags & connection_connected)
    {
        connection = msg->connection;
        // this method is called from ble_evt_connection_disconnected need to set exit code only
        // when we call this method from open_ble_device
        if (state == State::initial_connection)
        {
            exit_code = (int)GanglionLibNative::STATUS_OK;
        }
    }
}

void ble_evt_connection_disconnected (const struct ble_msg_connection_disconnected_evt_t *msg)
{
    // atempt to reconnect
    ble_cmd_gap_connect_direct (&connect_addr, gap_address_type_random, 40, 60, 100, 0);
}

// ble_evt_attclient_group_found and ble_evt_attclient_procedure_completed are called after the same
// command(ble_cmd_attclient_read_by_group_type)
void ble_evt_attclient_group_found (const struct ble_msg_attclient_group_found_evt_t *msg)
{
    if (msg->uuid.len == 0)
    {
        return;
    }
    uint16 uuid = (msg->uuid.data[1] << 8) | msg->uuid.data[0];
    if (uuid == GANGLION_SERVICE_UUID)
    {
        ganglion_handle_start = msg->start;
        ganglion_handle_end = msg->end;
    }
}

void ble_evt_attclient_procedure_completed (
    const struct ble_msg_attclient_procedure_completed_evt_t *msg)
{
    if (state == State::write_to_client_char)
    {
        if (msg->result == 0)
        {
            exit_code = (int)GanglionLibNative::STATUS_OK;
        }
    }
    if (state == State::open_called)
    {
        if ((ganglion_handle_start) && (ganglion_handle_end))
        {
            ble_cmd_attclient_find_information (msg->connection, ganglion_handle_start,
                ganglion_handle_end); // triggers ble_evt_attclient_find_information_found
        }
    }
    else if (state == State::config_called)
    {
        if (msg->result == 0)
        {
            exit_code = (int)GanglionLibNative::STATUS_OK;
        }
    }
}

// finds characteristic handles and set exit_code for open_ganglion call
void ble_evt_attclient_find_information_found (
    const struct ble_msg_attclient_find_information_found_evt_t *msg)
{
    if (state == State::open_called)
    {
        if (msg->uuid.len == 2)
        {
            uint16 uuid = (msg->uuid.data[1] << 8) | msg->uuid.data[0];
            if (uuid == CLIENT_CHARACTERISTIC_UUID)
            {
                client_char_handle = msg->chrhandle;
            }
        }
        if (msg->uuid.len == 16)
        {
            bool is_send = true;
            bool is_recv = true;
            for (int i = 0; i < 16; i++)
            {
                if (msg->uuid.data[i] != send_char_uuid_bytes[i])
                {
                    is_send = false;
                }
                if (msg->uuid.data[i] != recv_char_uuid_bytes[i])
                {
                    is_recv = false;
                }
            }
            if (is_recv)
            {
                ganglion_handle_recv = msg->chrhandle;
            }
            if (is_send)
            {
                ganglion_handle_send = msg->chrhandle;
            }
        }
        if ((ganglion_handle_send) && (ganglion_handle_recv) && (client_char_handle) &&
            (state == State::open_called))
        {
            exit_code = (int)GanglionLibNative::STATUS_OK;
        }
    }
}

void ble_evt_attclient_attribute_value (const struct ble_msg_attclient_attribute_value_evt_t *msg)
{
    if ((int)msg->value.len >= 18)
    {
        unsigned char values[20] = {0};
        memcpy (values, msg->value.data, msg->value.len * sizeof (unsigned char));
        struct GanglionLibNative::GanglionDataNative data (values, (long)get_timestamp ());
        data_queue.push (data);
    }
}
