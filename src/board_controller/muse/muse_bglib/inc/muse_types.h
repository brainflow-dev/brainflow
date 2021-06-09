#pragma once

#include <string.h>


// to pass two args to initialize
struct MuseInputData
{
    int timeout;
    char uart_port[1024];

    MuseInputData (int timeout, const char *uart_port)
    {
        this->timeout = timeout;
        strcpy (this->uart_port, uart_port);
    }

    MuseInputData (const MuseInputData &other)
    {
        timeout = other.timeout;
        strcpy (uart_port, other.uart_port);
    }
};