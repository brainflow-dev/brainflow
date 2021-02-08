#include "libftdi_serial.h"
#include "serial.h"


#ifdef USE_LIBFTDI
#include <chrono>
#include <ftdi.h>

#include "board.h"


LibFTDISerial::LibFTDISerial (const char *description, Board *board)
    : ftdi (ftdi_new ()), description (description), port_open (false), board (board)
{
}

LibFTDISerial::~LibFTDISerial ()
{
    if (port_open)
    {
        ftdi_usb_close (ftdi);
    }
    ftdi_free (ftdi);
}

bool LibFTDISerial::is_libftdi (const char *port_name)
{
    LibFTDISerial serial (port_name);
    int open_result = ftdi_usb_open_string (serial.ftdi, port_name);
    if (open_result == 0)
    {
        ftdi_usb_close (serial.ftdi);
    }
    return open_result != -11;
}

void LibFTDISerial::log_error (const char *action, const char *message)
{
    if (board != nullptr)
    {
        board->safe_logger (spdlog::level::err, "libftdi {}: {} -> {}", description, action,
            message ? message : ftdi_get_error_string (ftdi));
    }
}

int LibFTDISerial::open_serial_port ()
{
    // https://www.intra2net.com/en/developer/libftdi/documentation/ftdi_8c.html#aae805b82251a61dae46b98345cd84d5c
    switch (ftdi_usb_open_string (ftdi, description.c_str ()))
    {
        case 0:
            port_open = true;
            return (int)SerialExitCodes::OK;
        case -10:
            log_error ("open_serial_port ()");
            return (int)SerialExitCodes::CLOSE_ERROR;
        default:
            log_error ("open_serial_port ()");
            return (int)SerialExitCodes::OPEN_PORT_ERROR;
    }
}

bool LibFTDISerial::is_port_open ()
{
    return port_open;
}

int LibFTDISerial::set_serial_port_settings (int ms_timeout, bool timeout_only)
{
    int result;
    if (!timeout_only)
    {
        result = ftdi_set_line_property (ftdi, BITS_8, STOP_BIT_1, NONE);
        if (result != 0)
        {
            log_error ("set_serial_port_settings");
            return (int)SerialExitCodes::SET_PORT_STATE_ERROR;
        }
        result = set_custom_baudrate (115200);
        if (result != (int)SerialExitCodes::OK)
        {
            return result;
        }
        result = ftdi_setdtr_rts (ftdi, 1, 1);
        result |= ftdi_setflowctrl (ftdi, SIO_DISABLE_FLOW_CTRL);
        if (result != 0)
        {
            // -1 setting failed, -2 usb device unavailable
            log_error ("set_serial_port_settings");
            return (int)SerialExitCodes::SET_PORT_STATE_ERROR;
        }
    }

    ftdi->usb_read_timeout = ms_timeout;

    return (int)SerialExitCodes::OK;
}

int LibFTDISerial::set_custom_baudrate (int baudrate)
{
    switch (ftdi_set_baudrate (ftdi, baudrate))
    {
        case 0:
            return (int)SerialExitCodes::OK;
        case -3: // usb device unavailable
            log_error ("set_custom_baudrate");
            return (int)SerialExitCodes::OPEN_PORT_ERROR;
        default: // -1 invalid baudrate, -2 setting baudrate failed
            log_error ("set_custom_baudrate");
            return (int)SerialExitCodes::SET_PORT_STATE_ERROR;
    }
}

int LibFTDISerial::flush_buffer ()
{
#if FTDI_MAJOR_VERSION > 2 || (FTDI_MAJOR_VERSION == 1 && FTDI_MINOR_VERSIOM >= 5)
    // correct tcflush was added in libftdi 1.5
    switch (ftdi_tcioflush (ftdi))
    {
        case 0:
            return (int)SerialExitCodes::OK;
        case -3: // usb device unavailable
            log_error ("flush_buffer ()");
            return (int)SerialExitCodes::OPEN_PORT_ERROR;
        default: // -1,-2 chip failed to purge a buffer
            log_error ("flush_buffer ()");
    }
#else
    log_error ("flush_buffer ()", "libftdi version <=1.4, tcflush unimplemented");
#endif
    return (int)SerialExitCodes::SET_PORT_STATE_ERROR;
}

int LibFTDISerial::read_from_serial_port (void *bytes_to_read, int size)
{
    // the ftdi will send us data after its latency (max 255ms, default
    // 16ms) times out, even if its buffer is empty, despite the usb
    // timeout.  libftdi does not enforce waiting for the usb timeout if
    // the chip responds, even if the chip responds with an empty buffer.
    // so, the read is repeated until the timeout is reached.

    // this latency behavior is documented in
    // http://www.ftdichip.com/Support/Documents/AppNotes/AN232B-04_DataLatencyFlow.pdf

    auto deadline =
        std::chrono::steady_clock::now () + std::chrono::milliseconds (ftdi->usb_read_timeout);
    int bytes_read = 0;
    while (bytes_read == 0 && size > 0 && std::chrono::steady_clock::now () < deadline)
    {
        bytes_read = ftdi_read_data (ftdi, static_cast<unsigned char *> (bytes_to_read), size);
        // TODO: negative values are libusb error codes, -666 means usb device unavailable
        if (bytes_read < 0)
        {
            log_error ("read_from_serial_port");
        }
    }

    return bytes_read;
}

int LibFTDISerial::send_to_serial_port (const void *message, int length)
{
    int bytes_written =
        ftdi_write_data (ftdi, static_cast<unsigned const char *> (message), length);
    // TODO: negative values are libusb error codes, -666 means usb device unavailable
    if (bytes_written < 0)
    {
        log_error ("send_to_serial_port");
    }
    return bytes_written;
}

int LibFTDISerial::close_serial_port ()
{
    if (ftdi_usb_close (ftdi) == 0)
    {
        port_open = false;
        return (int)SerialExitCodes::OK;
    }
    else
    {
        log_error ("close_serial_port ()");
        return (int)SerialExitCodes::CLOSE_ERROR;
    }
}

const char *LibFTDISerial::get_port_name ()
{
    return description.c_str ();
}

#else

LibFTDISerial::LibFTDISerial (const char *description, Board *board)
{
}

LibFTDISerial::~LibFTDISerial ()
{
}

bool LibFTDISerial::is_libftdi (const char *port_name)
{
    return false;
}

int LibFTDISerial::open_serial_port ()
{
    return (int)SerialExitCodes::NO_LIBFTDI;
}

bool LibFTDISerial::is_port_open ()
{
    return false;
}

int LibFTDISerial::set_serial_port_settings (int ms_timeout, bool timeout_only)
{
    return (int)SerialExitCodes::NO_LIBFTDI;
}

int LibFTDISerial::set_custom_baudrate (int baudrate)
{
    return (int)SerialExitCodes::NO_LIBFTDI;
}

int LibFTDISerial::flush_buffer ()
{
    return (int)SerialExitCodes::NO_LIBFTDI;
}

int LibFTDISerial::read_from_serial_port (void *bytes_to_read, int size)
{
    return (int)SerialExitCodes::NO_LIBFTDI;
}

int LibFTDISerial::send_to_serial_port (const void *message, int length)
{
    return (int)SerialExitCodes::NO_LIBFTDI;
}

int LibFTDISerial::close_serial_port ()
{
    return (int)SerialExitCodes::NO_LIBFTDI;
}

const char *LibFTDISerial::get_port_name ()
{
    return "";
}

#endif
