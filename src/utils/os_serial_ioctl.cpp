#include <stdlib.h>
#include <string.h>

#include "os_serial.h"

// ioctl headers conflict with standard posix headers for serial port, ioctl needed for custom
// baudrate, move this function to another compilation unit from serial.cpp
#if defined(_WIN32)
#include <windows.h>

// linux, also need to check that system headers installed
#elif defined(__linux__) && !defined(__ANDROID__)
#if defined __has_include
#if __has_include(<sys/ioctl.h>) && __has_include(</usr/include/asm/ioctls.h>) && __has_include(</usr/include/asm/termbits.h>)
#include <sys/ioctl.h>

#include </usr/include/asm/ioctls.h>
#include </usr/include/asm/termbits.h>
#else
#define NO_IOCTL_HEADERS
#endif
#else
#define NO_IOCTL_HEADERS
#endif

#elif defined(__APPLE__)
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#endif


#if defined(_WIN32)
int OSSerial::set_custom_baudrate (int baudrate)
{
    DCB dcb_serial_params = {0};
    dcb_serial_params.DCBlength = sizeof (dcb_serial_params);
    if (GetCommState (this->port_descriptor, &dcb_serial_params) == 0)
    {
        CloseHandle (this->port_descriptor);
        return SerialExitCodes::GET_PORT_STATE_ERROR;
    }

    dcb_serial_params.BaudRate = baudrate;
    if (SetCommState (this->port_descriptor, &dcb_serial_params) == 0)
    {
        CloseHandle (this->port_descriptor);
        return SerialExitCodes::SET_PORT_STATE_ERROR;
    }
    return SerialExitCodes::OK;
}

#elif defined(__linux__) && !defined(__ANDROID__)
#ifdef NO_IOCTL_HEADERS
int OSSerial::set_custom_baudrate (int baudrate)
{
    return SerialExitCodes::NO_SYSTEM_HEADERS_FOUND_ERROR;
}
#else
int OSSerial::set_custom_baudrate (int baudrate)
{
    struct termios2 port_settings;
    memset (&port_settings, 0, sizeof (port_settings));

    ioctl (this->port_descriptor, TCGETS2, &port_settings);
    port_settings.c_cflag &= ~CBAUD;   // Remove current BAUD rate
    port_settings.c_cflag |= BOTHER;   // Allow custom BAUD rate using int input
    port_settings.c_ispeed = baudrate; // Set the input BAUD rate
    port_settings.c_ospeed = baudrate; // Set the output BAUD rate
    int r = ioctl (this->port_descriptor, TCSETS2, &port_settings);

    if (r == 0)
    {
        return SerialExitCodes::OK;
    }
    else
    {
        return SerialExitCodes::SET_PORT_STATE_ERROR;
    }
}
#endif

#elif defined(__APPLE__)
int OSSerial::set_custom_baudrate (int baudrate)
{
    struct termios port_settings;
    memset (&port_settings, 0, sizeof (port_settings));
    tcgetattr (this->port_descriptor, &port_settings);

    cfsetispeed (&port_settings, baudrate);
    cfsetospeed (&port_settings, baudrate);

    if (tcsetattr (this->port_descriptor, TCSANOW, &port_settings) != 0)
        return SerialExitCodes::SET_PORT_STATE_ERROR;
    tcflush (this->port_descriptor, TCIOFLUSH);
    return SerialExitCodes::OK;
}

#else
int OSSerial::set_custom_baudrate (int baudrate)
{
    return SerialExitCodes::SET_PORT_STATE_ERROR;
}
#endif
