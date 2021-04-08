#pragma once

#include <string.h>


namespace BrainBitBLEDLib
{
    // to pass two args to initialize
    struct BrainBitInputData
    {
        int timeout;
        char uart_port[1024];

        BrainBitInputData (int timeout, const char *uart_port)
        {
            this->timeout = timeout;
            strcpy (this->uart_port, uart_port);
        }

        BrainBitInputData (const BrainBitInputData &other)
        {
            timeout = other.timeout;
            strcpy (uart_port, other.uart_port);
        }
    };
}