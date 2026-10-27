#pragma once

#include <string>
#include <vector>

// Metadata only: enumeration must never open a port or send device commands.
struct SerialPortInfo
{
    std::string port;
    int vid = -1;
    int pid = -1;
    std::string serial_number;
};

enum SerialExitCodes : int
{
    OK = 0,
    OPEN_PORT_ERROR = -1,
    GET_PORT_STATE_ERROR = -2,
    SET_PORT_STATE_ERROR = -3,
    SET_TIMEOUT_ERROR = -4,
    CLOSE_ERROR = -5,
    NO_SYSTEM_HEADERS_FOUND_ERROR = -6,
    PORT_NAME_ERROR = -7,
    NO_LIBFTDI_ERROR = -8,
    ENUMERATION_ERROR = -9
};

class Board;

class Serial
{
public:
    static Serial *create (const char *port_name, Board *board = nullptr);
    // USB serial ports with OS device nodes. Does not enumerate libftdi URLs.
    // An empty successful result differs from an unsupported/failed enumeration.
    static int get_port_info (std::vector<SerialPortInfo> &ports);

    virtual ~Serial ()
    {
    }
    virtual int open_serial_port () = 0;
    virtual bool is_port_open () = 0;
    // timeout_only indicates to configure only the timeout, not the baud and line properties
    virtual int set_serial_port_settings (int ms_timeout = 1000, bool timeout_only = false) = 0;
    virtual int set_custom_baudrate (int baudrate) = 0;
    virtual int set_custom_latency (int latency = 1) = 0;
    virtual int flush_buffer () = 0;
    virtual int read_from_serial_port (void *bytes_to_read, int size) = 0;
    virtual int send_to_serial_port (const void *message, int length) = 0;
    virtual int close_serial_port () = 0;
    virtual const char *get_port_name () = 0;
};
