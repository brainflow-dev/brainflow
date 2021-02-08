#include <stdlib.h>
#include <string.h>
#include <string>


#include "os_serial.h"
#include "serial.h"

#ifdef _WIN32

OSSerial::OSSerial (const char *port_name)
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

bool OSSerial::is_port_open ()
{
    return (this->port_descriptor != NULL);
}

int OSSerial::open_serial_port ()
{
    this->port_descriptor =
        CreateFile (this->port_name, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (this->port_descriptor == INVALID_HANDLE_VALUE)
    {
        return SerialExitCodes::OPEN_PORT_ERROR;
    }
    return SerialExitCodes::OK;
}

int OSSerial::set_serial_port_settings (int ms_timeout, bool timeout_only)
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

int OSSerial::flush_buffer ()
{
    PurgeComm (this->port_descriptor, PURGE_TXCLEAR | PURGE_RXCLEAR);
    return SerialExitCodes::OK;
}

int OSSerial::read_from_serial_port (void *bytes_to_read, int size)
{
    DWORD readed;
    if (!ReadFile (this->port_descriptor, bytes_to_read, size, &readed, NULL))
    {
        return 0;
    }
    return (int)readed;
}

int OSSerial::send_to_serial_port (const void *message, int length)
{
    DWORD bytes_written;
    if (!WriteFile (this->port_descriptor, message, length, &bytes_written, NULL))
    {
        return 0;
    }
    return bytes_written;
}

int OSSerial::close_serial_port ()
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

OSSerial::OSSerial (const char *port_name)
{
    strcpy (this->port_name, port_name);
    port_descriptor = 0;
}

bool OSSerial::is_port_open ()
{
    return (this->port_descriptor > 0);
}

int OSSerial::open_serial_port ()
{
    int flags = O_RDWR | O_NOCTTY;
    this->port_descriptor = open (this->port_name, flags);
    if (this->port_descriptor < 0)
    {
        return SerialExitCodes::OPEN_PORT_ERROR;
    }
    return SerialExitCodes::OK;
}

int OSSerial::set_serial_port_settings (int ms_timeout, bool timeout_only)
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

int OSSerial::read_from_serial_port (void *bytes_to_read, int size)
{
    int res = read (this->port_descriptor, bytes_to_read, size);
    if (res < 0)
    {
        return 0;
    }
    return res;
}

int OSSerial::flush_buffer ()
{
    tcflush (this->port_descriptor, TCIOFLUSH);
    return SerialExitCodes::OK;
}

int OSSerial::send_to_serial_port (const void *message, int length)
{
    int res = write (this->port_descriptor, message, length);
    return res;
}

int OSSerial::close_serial_port ()
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
