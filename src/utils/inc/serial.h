#pragma once

#ifdef _WIN32
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif
#include <stdlib.h>
#include <string.h>


enum SerialExitCodes
{
    OK = 0,
    OPEN_PORT_ERROR = -1,
    GET_PORT_STATE_ERROR = -2,
    SET_PORT_STATE_ERROR = -3,
    SET_TIMEOUT_ERROR = -4,
    CLOSE_ERROR = -5
};

class Serial
{

public:
    Serial (const char *port_name);
    ~Serial ()
    {
        close_serial_port ();
    }

    int open_serial_port ();
    bool is_port_open ();
    int set_serial_port_settings (int ms_timeout = 1000);
    int read_from_serial_port (void *bytes_to_read, int size);
    int send_to_serial_port (const void *message, int length);
    int close_serial_port ();
    char *get_port_name ()
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
