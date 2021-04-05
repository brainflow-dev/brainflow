#pragma once

#include <ctype.h>
#include <string>

#include "cmd_def.h"
#include "uart.h"

#define UART_TIMEOUT 1000
#define BRAINBIT_BLED_DATA_SIZE 8

namespace BrainBitBLEDLib
{
    enum State
    {
        NONE = 0,
        INIT_CALLED = 1,
        INITIAL_CONNECTION = 2,
        OPEN_CALLED = 3,
        CONFIG_CALLED = 4,
        CLOSE_CALLED = 5,
        GET_DATA_CALLED = 6,
        WRITE_TO_CLIENT_CHAR = 7
    };

    void output (uint8 len1, uint8 *data1, uint16 len2, uint8 *data2);
    int read_message (int timeout_ms);
    int open_ble_dev ();
    int wait_for_callback (int num_seconds);
    int reset_ble_dev ();
    int config_board (uint8 *command, int len);
}