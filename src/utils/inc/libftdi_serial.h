#pragma once

#include <string>

#include "serial.h"

#ifdef USE_LIBFTDI
#include <ftdi.h>
#endif

class LibFTDISerial : public Serial
{

public:
    LibFTDISerial (const char *description, Board *board = nullptr);
    virtual ~LibFTDISerial ();

    int open_serial_port ();
    bool is_port_open ();
    int set_serial_port_settings (int ms_timeout = 1000, bool timeout_only = false);
    int set_custom_baudrate (int baudrate);
    int flush_buffer ();
    int read_from_serial_port (void *bytes_to_read, int size);
    int send_to_serial_port (const void *message, int length);
    int close_serial_port ();
    const char *get_port_name ();

private:
#ifdef USE_LIBFTDI
    void log_error (const char *action, const char *message = nullptr);

    struct ftdi_context ftdi;
    std::string description;
    int last_result;
    bool port_open;
    bool lib_init;
    Board *board;
#endif
};
