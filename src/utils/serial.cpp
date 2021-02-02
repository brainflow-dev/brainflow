#include <string>

#include "serial.h"

Serial::~Serial ()
{
}

/////////////////////////////////////////////////
/////////////////// LibFTDI /////////////////////
/////////////////////////////////////////////////

#ifdef USE_LIBFTDIPP

// Note: LibFTDI also supports listing all connected devies.  This is not used yet.

#include <chrono>
#include <thread>

#include <ftdi.hpp>
#include <spdlog/spdlog.h>

class LibFTDISerial : public Serial
{
public:
    // "d:<devicenode>"
    //      path of bus and device-node (e.g. "003/001") within usb device tree (usually at
    //      /proc/bus/usb)
    // "i:<vendor>:<product>"
    //      first device with given vendor and product id, ids can be decimal, octal (preceded by
    //      "0") or hex (preceded by "0x")
    // "i:<vendor>:<product>:<index>"
    //      as above with index being the number of the device (starting with 0) if there are more
    //      than one
    // "s:<vendor>:<product>:<serial>"
    //      first device with given vendor id, product id and serial string
    // there are also other ftdi constructors that take these items without parsing a string
    LibFTDISerial (const char *description)
        : description (description), port_open (false), logger (spdlog::get ("brainflow_logger"))
    {
    }

    static bool is_libftdi (const char *port_name)
    {
        LibFTDISerial test (port_name);
        return test.ctx.open (port_name) != -11;
    }

    void log_error (const char *action, const char *message = nullptr)
    {
        logger->error ("libftdi {}: {} -> {}", description, action, message ? message : ctx.error_string ());
    }

    int open_serial_port ()
    {
        // https://www.intra2net.com/en/developer/libftdi/documentation/ftdi_8c.html#aae805b82251a61dae46b98345cd84d5c
        switch (ctx.open (description))
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

    bool is_port_open ()
    {
        return port_open;
    }

    int set_serial_port_settings (int ms_timeout = 1000, bool timeout_only = false)
    {
        int result;
        if (!timeout_only)
        {
            result = ctx.set_line_property (BITS_8, STOP_BIT_1, NONE);
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
            result = ctx.set_modem_control (Ftdi::Context::Dtr | Ftdi::Context::Rts);
            result |= ctx.set_flow_control (SIO_DISABLE_FLOW_CTRL);
            if (result != 0)
            {
                // -1 setting failed, -2 usb device unavailable
                log_error ("set_serial_port_settings");
                return (int)SerialExitCodes::SET_PORT_STATE_ERROR;
            }
        }

        ctx.set_usb_read_timeout (ms_timeout);

        return (int)SerialExitCodes::OK;
    }

    int set_custom_baudrate (int baudrate)
    {
        switch (ctx.set_baud_rate (115200))
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

    int flush_buffer ()
    {
#if FTDI_MAJOR_VERSION > 2 || (FTDI_MAJOR_VERSION == 1 && FTDI_MINOR_VERSIOM >= 5)
        // correct tcflush was added in libftdi 1.5
        switch (ctx.tcflush (Ftdi::Context::Input | Ftdi::Context::Output))
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

    int read_from_serial_port (void *bytes_to_read, int size)
    {
        // the ftdi will send us data after its latency (max 255ms, default
        // 16ms) times out, even if its buffer is empty, despite the usb
        // timeout.  libftdi does not enforce waiting for the usb timeout if
        // the chip responds, even if the chip responds with an empty buffer.
        // so, the read is repeated until the timeout is reached.

        // this latency behavior is documented in
        // http://www.ftdichip.com/Support/Documents/AppNotes/AN232B-04_DataLatencyFlow.pdf

        auto deadline = std::chrono::steady_clock::now () +
            std::chrono::milliseconds (ctx.get_usb_read_timeout ());
        int bytes_read = 0;
        while (bytes_read == 0 && size > 0 && std::chrono::steady_clock::now () < deadline)
        {
            bytes_read = ctx.read (static_cast<unsigned char *> (bytes_to_read), size);
            // TODO: negative values are libusb error codes, -666 means usb device unavailable
            if (bytes_read < 0)
            {
                log_error ("read_from_serial_port");
            }
        }

        return bytes_read;
    }

    int send_to_serial_port (const void *message, int length)
    {
        int bytes_written = ctx.write (static_cast<unsigned const char *> (message), length);
        // TODO: negative values are libusb error codes, -666 means usb device unavailable
        if (bytes_written < 0)
        {
            log_error ("send_to_serial_port");
        }
        return bytes_written;
    }

    int close_serial_port ()
    {
        if (ctx.close () == 0)
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

    const char *get_port_name ()
    {
        return description.c_str ();
    }

private:
    Ftdi::Context ctx;
    std::string description;
    bool port_open;
    std::shared_ptr<spdlog::logger> logger;
};

#endif

// For platform-specific definition code
#define Serial OSSerial

/////////////////////////////////////////////////
/////////////////// Windows /////////////////////
/////////////////////////////////////////////////

#ifdef _WIN32

#include <windows.h>

Serial::Serial (const char *port_name)
{
    std::string port_name_string (port_name);
    // add winapi specific prefix for port name if not provided
    if (port_name_string.find (std::string ("COM")) == 0)
    {
        port_name_string = std::string ("\\\\.\\") + port_name_string;
    }
    strcpy (this->port_name, port_name_string.c_str ());
    port_descriptor = NULL;
}

bool Serial::is_port_open ()
{
    return (this->port_descriptor != NULL);
}

int Serial::open_serial_port ()
{
    this->port_descriptor =
        CreateFile (this->port_name, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (this->port_descriptor == INVALID_HANDLE_VALUE)
        return SerialExitCodes::OPEN_PORT_ERROR;
    return SerialExitCodes::OK;
}

int Serial::set_serial_port_settings (int ms_timeout, bool timeout_only)
{
    if (!timeout_only)
    {
        DCB dcb_serial_params = {0};
        dcb_serial_params.DCBlength = sizeof (dcb_serial_params);
        if (GetCommState (this->port_descriptor, &dcb_serial_params) == 0)
        {
            CloseHandle (this->port_descriptor);
            return SerialExitCodes::GET_PORT_STATE_ERROR;
        }

        dcb_serial_params.BaudRate = CBR_115200;
        dcb_serial_params.ByteSize = 8;
        dcb_serial_params.StopBits = ONESTOPBIT;
        dcb_serial_params.Parity = NOPARITY;
        if (SetCommState (this->port_descriptor, &dcb_serial_params) == 0)
        {
            CloseHandle (this->port_descriptor);
            return SerialExitCodes::SET_PORT_STATE_ERROR;
        }
    }

    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout = ms_timeout;
    timeouts.ReadTotalTimeoutConstant = ms_timeout;
    timeouts.ReadTotalTimeoutMultiplier = 100;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;
    if (SetCommTimeouts (this->port_descriptor, &timeouts) == 0)
    {
        CloseHandle (this->port_descriptor);
        return SerialExitCodes::SET_TIMEOUT_ERROR;
    }
    return SerialExitCodes::OK;
}

int Serial::flush_buffer ()
{
    PurgeComm (this->port_descriptor, PURGE_TXCLEAR | PURGE_RXCLEAR);
    return SerialExitCodes::OK;
}

int Serial::read_from_serial_port (void *bytes_to_read, int size)
{
    DWORD readed;
    if (!ReadFile (this->port_descriptor, bytes_to_read, size, &readed, NULL))
        return 0;
    return (int)readed;
}

int Serial::send_to_serial_port (const void *message, int length)
{
    DWORD bytes_written;
    if (!WriteFile (this->port_descriptor, message, length, &bytes_written, NULL))
        return 0;
    return bytes_written;
}

int Serial::close_serial_port ()
{
    if (this->is_port_open ())
    {
        CloseHandle (this->port_descriptor);
        this->port_descriptor = NULL;
    }
    return SerialExitCodes::OK;
}

/////////////////////////////////////////////////
//////////////////// Linux //////////////////////
/////////////////////////////////////////////////

#else

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

Serial::Serial (const char *port_name)
{
    strcpy (this->port_name, port_name);
    port_descriptor = 0;
}

bool Serial::is_port_open ()
{
    return (this->port_descriptor > 0);
}

int Serial::open_serial_port ()
{
    int flags = O_RDWR | O_NOCTTY;
    this->port_descriptor = open (this->port_name, flags);
    if (this->port_descriptor < 0)
        return SerialExitCodes::OPEN_PORT_ERROR;
    return SerialExitCodes::OK;
}

int Serial::set_serial_port_settings (int ms_timeout, bool timeout_only)
{
    struct termios port_settings;
    memset (&port_settings, 0, sizeof (port_settings));

    tcgetattr (this->port_descriptor, &port_settings);
    if (!timeout_only)
    {
        cfsetispeed (&port_settings, B115200);
        cfsetospeed (&port_settings, B115200);
        port_settings.c_cflag &= ~PARENB;
        port_settings.c_cflag &= ~CSTOPB;
        port_settings.c_cflag &= ~CSIZE;
        port_settings.c_cflag |= CS8;
        port_settings.c_cflag |= CREAD;
        port_settings.c_cflag |= CLOCAL;
        port_settings.c_cflag |= HUPCL;
        port_settings.c_iflag = IGNPAR;
        port_settings.c_iflag &= ~(ICANON | IXOFF | IXON | IXANY);
        port_settings.c_oflag = 0;
        port_settings.c_lflag = 0;
    }
    port_settings.c_cc[VMIN] = 0;
    port_settings.c_cc[VTIME] = ms_timeout / 100; // vtime is in tenths of a second

    if (tcsetattr (this->port_descriptor, TCSANOW, &port_settings) != 0)
        return SerialExitCodes::SET_PORT_STATE_ERROR;
    tcflush (this->port_descriptor, TCIOFLUSH);
    return SerialExitCodes::OK;
}

int Serial::read_from_serial_port (void *bytes_to_read, int size)
{
    int res = read (this->port_descriptor, bytes_to_read, size);
    if (res < 0)
    {
        return 0;
    }
    return res;
}

int Serial::flush_buffer ()
{
    tcflush (this->port_descriptor, TCIOFLUSH);
    return SerialExitCodes::OK;
}

int Serial::send_to_serial_port (const void *message, int length)
{
    int res = write (this->port_descriptor, message, length);
    return res;
}

int Serial::close_serial_port ()
{
    if (this->is_port_open ())
    {
        int res = close (port_descriptor);
        port_descriptor = 0;
        if (res < 0)
        {
            return SerialExitCodes::CLOSE_ERROR;
        }
    }
    return SerialExitCodes::OK;
}

#endif

// End platform-specific definition code
#undef Serial

/////////////////////////////////////////////////
/////////////// Factory Function ////////////////
/////////////////////////////////////////////////

Serial *Serial::create (const char *port_name)
{
#ifdef USE_LIBFTDIPP
    if (LibFTDISerial::is_libftdi (port_name))
    {
        return new LibFTDISerial (port_name);
    }
#endif

    return new OSSerial (port_name);
}
