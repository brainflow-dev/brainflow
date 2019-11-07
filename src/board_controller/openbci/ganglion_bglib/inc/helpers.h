#pragma once

#include <ctype.h>
#include <string>

#include "cmd_def.h"
#include "uart.h"

#define UART_TIMEOUT 1000

namespace GanglionLib
{
    enum State
    {
        none = 0,
        init_called = 1,
        initial_connection = 2,
        open_called = 3,
        config_called = 4,
        close_called = 5,
        get_data_called = 6,
        write_to_client_char = 7
    };

    void output (uint8 len1, uint8 *data1, uint16 len2, uint8 *data2);
    int read_message (int timeout_ms);
    int open_ble_dev ();
    int wait_for_callback (int num_attempts);
    int reset_ble_dev ();
}