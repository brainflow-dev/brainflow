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
    rep[0] = -1;
    rep[1] = -1;
}

int SocketBluetooth::connect ()
{
    if (pipe (rep) == -1)
    {
        return (int)SocketBluetoothReturnCodes::OS_SPECIFIC_ERROR;
    }
    int flags = fcntl (rep[0], F_GETFL, 0);
    fcntl (rep[0], F_SETFL, flags | O_NONBLOCK);
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

    fd_set set;
    FD_ZERO (&set);
    FD_SET (socket_bt, &set);
    FD_SET (rep[0], &set);
    int nfds = (socket_bt > rep[0]) ? socket_bt : rep[0];

    if (pselect (nfds + 1, &set, NULL, NULL, NULL, NULL) >= 0)
    {
        if (FD_ISSET (socket_bt, &set))
        {
            int res = ::recv (socket_bt, data, size, 0);
            for (int i = 0; i < res; i++)
            {
                temp_buffer.push (data[i]);
            }
        }
    }
    if ((int)temp_buffer.size () < size)
    {
        return 0;
    }
    for (int i = 0; i < size; i++)
    {
        data[i] = temp_buffer.front ();
        temp_buffer.pop ();
    }
    return size;
}

int SocketBluetooth::close ()
{
    int result = ::close (socket_bt);
    socket_bt = -1;
    return result == 0 ? (int)SocketBluetoothReturnCodes::STATUS_OK :
                         (int)SocketBluetoothReturnCodes::DISCONNECT_ERROR;
}

std::pair<std::string, int> SocketBluetooth::discover (char *selector)
{
    return std::make_pair<std::string, int> (
        "", (int)SocketBluetoothReturnCodes::UNIMPLEMENTED_ERROR);
}