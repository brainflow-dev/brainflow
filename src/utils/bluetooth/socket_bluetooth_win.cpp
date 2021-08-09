#include "bluetooth_functions.h"
#include "socket_bluetooth.h"
#include <iostream>
#undef UNICODE
#undef _UNICODE

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "bthprops.lib")

SocketBluetooth::SocketBluetooth (std::string mac_addr, int port)
{
    this->mac_addr = mac_addr;
    this->port = port;
    socket_bt = INVALID_SOCKET;
}

int SocketBluetooth::connect ()
{
    WSADATA wsadata;
    int res = WSAStartup (MAKEWORD (2, 2), &wsadata);
    if (res != 0)
    {
        return (int)SocketBluetoothReturnCodes::WSA_STARTUP_ERROR;
    }

    int status = SOCKET_ERROR;
    socket_bt = socket (AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
    if (socket_bt == INVALID_SOCKET)
    {
        return (int)SocketBluetoothReturnCodes::CREATE_SOCKET_ERROR;
    }
    SOCKADDR_BTH addr = {0};
    int addr_size = sizeof (SOCKADDR_BTH);
    char mac_addr_c[40];
    strncpy (mac_addr_c, mac_addr.c_str (), 40);
    status = WSAStringToAddress (mac_addr_c, AF_BTH, NULL, (LPSOCKADDR)&addr, &addr_size);
    if (status == SOCKET_ERROR)
    {
        close ();
        return (int)SocketBluetoothReturnCodes::WSA_ADDR_ERROR;
    }
    addr.port = this->port;

    status = ::connect (socket_bt, (LPSOCKADDR)&addr, addr_size);
    if (status == SOCKET_ERROR)
    {
        close ();
        return (int)SocketBluetoothReturnCodes::CONNECT_ERROR;
    }
    unsigned long enable_non_blocking = 1;
    status = ioctlsocket (socket_bt, FIONBIO, &enable_non_blocking);

    if (status == SOCKET_ERROR)
    {
        close ();
        return (int)SocketBluetoothReturnCodes::IOCTL_ERROR;
    }
    return (int)SocketBluetoothReturnCodes::STATUS_OK;
}

int SocketBluetooth::send (const char *data, int size)
{
    if (socket_bt == INVALID_SOCKET)
    {
        return -1;
    }
    int res = ::send (socket_bt, data, size, 0);
    if (res == SOCKET_ERROR)
    {
        return -1;
    }
    return res;
}

int SocketBluetooth::recv (char *data, int size)
{
    if (socket_bt == INVALID_SOCKET)
    {
        return -1;
    }
    // waiting for exact amount of bytes
    int e = bytes_available ();
    std::cout << e << std::endl;
    if (e < size)
    {
        // return 0;
    }
    fd_set set;
    FD_ZERO (&set);
    FD_SET (socket_bt, &set);

    int res = -1;

    timeval timeout {0, 0};
    if (select (1, &set, nullptr, nullptr, &timeout) >= 0)
    {
        if (FD_ISSET (socket_bt, &set))
        {
            res = ::recv (socket_bt, data, size, 0);
        }
    }
    return res;
}

int SocketBluetooth::bytes_available ()
{
    if (socket_bt == INVALID_SOCKET)
    {
        return -1;
    }
    u_long count;
    ioctlsocket (socket_bt, FIONREAD, &count);
    return count;
}

int SocketBluetooth::close ()
{
    closesocket (socket_bt);
    socket_bt = INVALID_SOCKET;
    WSACleanup ();
    return (int)SocketBluetoothReturnCodes::STATUS_OK;
}