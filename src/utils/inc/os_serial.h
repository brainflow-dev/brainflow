#pragma once

#ifdef _WIN32
#include <windows.h>
#endif

#include "serial.h"


class OSSerial : public Serial
{

public:
    OSSerial (const char *port_name);
    virtual ~OSSerial ()
    {
        close_serial_port ();
    }

    int open_serial_port ();
    bool is_port_open ();
    int set_serial_port_settings (int ms_timeout = 1000, bool timeout_only = false);
    int set_custom_baudrate (int baudrate);
    int flush_buffer ();
    int read_from_serial_port (void *bytes_to_read, int size);
    int send_to_serial_port (const void *message, int length);
    int close_serial_port ();
    const char *get_port_name ()
    {
        return port_name;
    }

private:
    char port_name[1024];
#ifdef _WIN32
    HANDLE port_descriptor;
#else
    int port_descriptor;
#endif
};
