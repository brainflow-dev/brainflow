#include <ctype.h>
#include <stdlib.h>

#include "cmd_def.h"
#include "muse_bglib_helper.h"

extern MuseBGLibHelper *helper;


void ble_evt_connection_status (const struct ble_msg_connection_status_evt_t *msg)
{
    if (helper != NULL)
    {
        helper->ble_evt_connection_status (msg);
    }
}

void ble_evt_connection_disconnected (const struct ble_msg_connection_disconnected_evt_t *msg)
{
    if (helper != NULL)
    {
        helper->ble_evt_connection_disconnected (msg);
    }
}

void ble_evt_attclient_group_found (const struct ble_msg_attclient_group_found_evt_t *msg)
{
    if (helper != NULL)
    {
        helper->ble_evt_attclient_group_found (msg);
    }
}

void ble_evt_attclient_procedure_completed (
    const struct ble_msg_attclient_procedure_completed_evt_t *msg)
{

    if (helper != NULL)
    {
        helper->ble_evt_attclient_procedure_completed (msg);
    }
}

void ble_evt_attclient_find_information_found (
    const struct ble_msg_attclient_find_information_found_evt_t *msg)
{
    if (helper != NULL)
    {
        helper->ble_evt_attclient_find_information_found (msg);
    }
}

void ble_evt_attclient_attribute_value (const struct ble_msg_attclient_attribute_value_evt_t *msg)
{
    if (helper != NULL)
    {
        helper->ble_evt_attclient_attribute_value (msg);
    }
}

void ble_evt_gap_scan_response (const struct ble_msg_gap_scan_response_evt_t *msg)
{
    if (helper != NULL)
    {
        helper->ble_evt_gap_scan_response (msg);
    }
}