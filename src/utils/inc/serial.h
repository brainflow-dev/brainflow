#pragma once

#ifdef _WIN32
#include <windows.h>
#endif
#include <stdlib.h>
#include <string.h>


enum SerialExitCodes : int
{
    OK = 0,
    OPEN_PORT_ERROR = -1,
    GET_PORT_STATE_ERROR = -2,
    SET_PORT_STATE_ERROR = -3,
    SET_TIMEOUT_ERROR = -4,
    CLOSE_ERROR = -5,
    NO_SYSTEM_HEADERS_FOUND_ERROR = -6
};

class Board;

class Serial
{
public:
    static Serial *create (const char *port_name, Board * board = nullptr);

    virtual ~Serial () = 0;
    virtual int open_serial_port () = 0;
    virtual bool is_port_open () = 0;
    // timeout_only indicates to configure only the timeout, not the baud and line properties
    virtual int set_serial_port_settings (int ms_timeout = 1000, bool timeout_only = false) = 0;
    virtual int set_custom_baudrate (int baudrate) = 0;
    virtual int flush_buffer () = 0;
    virtual int read_from_serial_port (void *bytes_to_read, int size) = 0;
    virtual int send_to_serial_port (const void *message, int length) = 0;
    virtual int close_serial_port () = 0;
    virtual const char *get_port_name () = 0;
};

// This class provides a home for the
// operating-system-specific implementations.

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
