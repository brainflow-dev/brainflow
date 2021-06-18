#pragma once

#include <string.h>


class MuseInputData
{
public:
    int board_id;
    int timeout;
    char uart_port[1024];
    char mac_addr[1024];
    bool use_mac_addr;
    char device_name[1024];
    bool use_device_name;

    MuseInputData (int timeout, const char *uart_port, int board_id)
    {
        this->board_id = board_id;
        this->timeout = timeout;
        strcpy (this->uart_port, uart_port);
        use_mac_addr = false;
        use_device_name = false;
    }

    MuseInputData (const MuseInputData &other)
    {
        timeout = other.timeout;
        board_id = other.board_id;
        strcpy (uart_port, other.uart_port);
        use_device_name = other.use_device_name;
        use_mac_addr = other.use_mac_addr;
        if (use_device_name)
        {
            strcpy (device_name, other.device_name);
        }
        if (use_device_name)
        {
            strcpy (mac_addr, other.mac_addr);
        }
    }

    void set_mac_addr (const char *mac_addr)
    {
        use_mac_addr = true;
        strcpy (this->mac_addr, mac_addr);
    }

    void set_device_name (const char *device_name)
    {
        use_device_name = true;
        strcpy (this->device_name, device_name);
    }
};
