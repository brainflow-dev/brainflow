#ifndef SERIAL
#define SERIAL

#ifdef _WIN32
#include <windows.h>
#else
#include <termios.h>
#endif
#include <stdlib.h>

#ifdef _WIN32
inline bool is_port_open (HANDLE port_descriptor)
{
	return (port_descriptor != NULL);
}

inline int open_serial_port (char *port_name, HANDLE *port_descriptor)
{
	*port_descriptor = CreateFile (port_name, GENERIC_READ|GENERIC_WRITE, 0, NULL,
        							OPEN_EXISTING, 0, NULL);
    if (port_descriptor == INVALID_HANDLE_VALUE)
    	return -1;
    return 0;
}

inline int set_serial_port_settings (HANDLE port_descriptor)
{
    DCB dcb_serial_params = {0};
    COMMTIMEOUTS timeouts = {0};
	dcb_serial_params.DCBlength = sizeof (dcb_serial_params);
    if (GetCommState (port_descriptor, &dcb_serial_params) == 0)
    {

        CloseHandle (port_descriptor);
        return -1;
    }

    dcb_serial_params.BaudRate = CBR_115200;
    dcb_serial_params.ByteSize = 8;
    dcb_serial_params.StopBits = ONESTOPBIT;
    dcb_serial_params.Parity = NOPARITY;
    if (SetCommState (port_descriptor, &dcb_serial_params) == 0)
    {
        CloseHandle (port_descriptor);
        return -2;
    }

    timeouts.ReadIntervalTimeout = 1000;
    timeouts.ReadTotalTimeoutConstant = 1000;
    timeouts.ReadTotalTimeoutMultiplier = 100;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;
    if (SetCommTimeouts (port_descriptor, &timeouts) == 0)
    {
        CloseHandle (port_descriptor);
        return -3;
    }
    return 0;
}

inline int read_from_serial_port (HANDLE port_descriptor, void *b, int size)
{
	DWORD readed;
	if (!ReadFile (port_descriptor, b, size, &readed, NULL))
		return 0;
	return (int) readed;
}

// force one byte
inline int send_to_serial_port (const void *message, HANDLE port_descriptor)
{
	DWORD bytes_written;
    if (!WriteFile (port_descriptor, message, 1, &bytes_written, NULL))
        return 0;
    return 1;
}

inline int close_serial_port (HANDLE port_descriptor)
{
    CloseHandle (port_descriptor);
    return 0;
}
#else
inline bool is_port_open (int port_descriptor)
{
	return (port_descriptor > 0);
}

inline int open_serial_port (char *port_name, int *port_descriptor)
{
	int flags = O_RDWR | O_NOCTTY;
    *port_descriptor = open (port_name, flags);
    if (*port_descriptor < 0)
        return -1;
    return 0;
}

inline int set_serial_port_settings (int port_descriptor)
{
	struct termios port_settings;
	memset (&port_settings, 0, sizeof (port_settings));

    tcgetattr (port_descriptor, &port_settings);
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
    // blocking read with timeout 1 sec
    port_settings.c_cc[VMIN] = 0;
    port_settings.c_cc[VTIME] = 10;

    if (tcsetattr (port_descriptor, TCSANOW, &port_settings) != 0)
        return -1;
    tcflush (port_descriptor, TCIOFLUSH);
    return 0;
}

inline int read_from_serial_port (int port_descriptor, void *b, int size)
{
    return read (port_descriptor, b, size);
}

// force sending just one byte
inline int send_to_serial_port (const void *message, int port_descriptor)
{
    int res = write (port_descriptor, message, 1);
    return res;
}

inline int close_serial_port (int port_descriptor)
{
    int res = close (port_descriptor);
    if (res < 0)
        return -1;
    return 0;
}
#endif
#endif
