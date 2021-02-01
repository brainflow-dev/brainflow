#pragma once

#include "shared_export.h"
#include <string.h>

namespace BitalinoLib
{
#pragma pack(push, 1)
    struct BitalinoData
    {
        static const int SIZE = 12;

        double data[SIZE];

        BitalinoData (double *data)
        {
            memcpy (this->data, data, sizeof (double) * SIZE);
        }

        BitalinoData ()
        {
        }

        BitalinoData (const BitalinoData &other)
        {
            memcpy (data, other.data, sizeof (double) * SIZE);
        }
    };

    // just to pass two args to initialize
    struct BitalinoInputData
    {
        int timeout;
        char uart_port[1024];

        BitalinoInputData (int timeout, const char *uart_port)
        {
            this->timeout = timeout;
            strcpy (this->uart_port, uart_port);
        }

        BitalinoInputData (const BitalinoInputData &other)
        {
            timeout = other.timeout;
            strcpy (uart_port, other.uart_port);
        }
    };

#pragma pack(pop)

    enum CustomExitCodes
    {
        STATUS_OK = 0,
        BITALINO_NOT_FOUND_ERROR = 1,
        BITALINO_IS_NOT_OPEN_ERROR = 2,
        BITALINO_ALREADY_PAIR_ERROR = 3,
        BITALINO_ALREADY_OPEN_ERROR = 4,
        SERVICE_NOT_FOUND_ERROR = 5,
        SEND_CHARACTERISTIC_NOT_FOUND_ERROR = 6,
        RECEIVE_CHARACTERISTIC_NOT_FOUND_ERROR = 7,
        DISCONNECT_CHARACTERISTIC_NOT_FOUND_ERROR = 8,
        TIMEOUT_ERROR = 9,
        STOP_ERROR = 10,
        FAILED_TO_SET_CALLBACK_ERROR = 11,
        FAILED_TO_UNSUBSCRIBE_ERROR = 12,
        GENERAL_ERROR = 13,
        NO_DATA_ERROR = 14,
        SYNC_ERROR = 15,
        NOT_IMPLEMENTED_ERROR = 16,
        INVALID_MAC_ADDR_ERROR = 17,
        PORT_OPEN_ERROR = 18
    };
}