#include "bluetooth_functions.h"
#include "bluetooth_types.h"
#include "socket_bluetooth.h"

#include <fcntl.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>


SocketBluetooth::SocketBluetooth (std::string mac_addr, int port)
{
    this->mac_addr = mac_addr;
    this->port = port;
    socket_bt = -1;
}

int SocketBluetooth::connect ()
{
    struct sockaddr_rc addr;
    memset (&addr, 0, sizeof (addr));

    socket_bt = socket (AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    addr.rc_family = AF_BLUETOOTH;
    addr.rc_channel = (uint8_t)port;
    str2ba (mac_addr.c_str (), &addr.rc_bdaddr);

    int status = ::connect (socket_bt, (struct sockaddr *)&addr, sizeof (addr));
    if (status != 0)
    {
        return (int)SocketBluetoothReturnCodes::CONNECT_ERROR;
    }

    int sock_flags = fcntl (socket_bt, F_GETFL, 0);
    fcntl (socket_bt, F_SETFL, sock_flags | O_NONBLOCK);

    return (int)SocketBluetoothReturnCodes::STATUS_OK;
}

int SocketBluetooth::send (const char *data, int size)
{
    if (socket_bt < 0)
    {
        return -1;
    }
    int res = ::send (socket_bt, data, size, 0);
    return res;
}

int SocketBluetooth::recv (char *data, int size)
{
    if (socket_bt < 0)
    {
        return -1;
    }
    // waiting for exact amount of bytes
    int e = bytes_available ();
    if (e < size)
    {
        return 0;
    }

    fd_set set;
    FD_ZERO (&set);
    FD_SET (socket_bt, &set);

    int res = -1;
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    if (select (socket_bt + 1, &set, NULL, NULL, &timeout) >= 0)
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
    if (socket_bt < 0)
    {
        return -1;
    }
    int count;
    ioctl (socket_bt, FIONREAD, &count);
    return count;
}

int SocketBluetooth::close ()
{
    ::close (socket_bt);
    socket_bt = -1;
    return (int)SocketBluetoothReturnCodes::STATUS_OK;
}

std::pair<std::string, int> SocketBluetooth::discover (char *selector)
{
    return std::make_pair<std::string, int> (
        "", (int)SocketBluetoothReturnCodes::UNIMPLEMENTED_ERROR);
}