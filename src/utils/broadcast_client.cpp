#include <string.h>

#include "broadcast_client.h"

///////////////////////////////
/////////// WINDOWS ///////////
//////////////////////////////
#ifdef _WIN32

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")


BroadCastClient::BroadCastClient (int port)
{
    this->port = port;
    connect_socket = INVALID_SOCKET;
    memset (&socket_addr, 0, sizeof (socket_addr));
}

int BroadCastClient::init ()
{
    WSADATA wsadata;
    int res = WSAStartup (MAKEWORD (2, 2), &wsadata);
    if (res != 0)
    {
        return (int)BroadCastClientReturnCodes::WSA_STARTUP_ERROR;
    }
    socket_addr.sin_family = AF_INET;
    socket_addr.sin_port = htons (port);
    socket_addr.sin_addr.s_addr = htonl (INADDR_ANY);
    connect_socket = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (connect_socket == INVALID_SOCKET)
    {
        return (int)BroadCastClientReturnCodes::CREATE_SOCKET_ERROR;
    }

    // ensure that library will not hang in blocking recv/send call
    DWORD timeout = 5000;
    setsockopt (connect_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof (timeout));
    setsockopt (connect_socket, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof (timeout));
    DWORD broadcast = 1;
    setsockopt (connect_socket, SOL_SOCKET, SO_BROADCAST, (char *)&broadcast, sizeof (broadcast));
    if (bind (connect_socket, (const struct sockaddr *)&socket_addr, sizeof (socket_addr)) != 0)
    {
        return (int)BroadCastClientReturnCodes::INIT_ERROR;
    }

    return (int)BroadCastClientReturnCodes::STATUS_OK;
}

int BroadCastClient::recv (void *data, int size)
{
    int len = sizeof (socket_addr);
    int res = recvfrom (connect_socket, (char *)data, size, 0, (sockaddr *)&socket_addr, &len);
    if (res == SOCKET_ERROR)
    {
        return -1;
    }
    return res;
}

void BroadCastClient::close ()
{
    closesocket (connect_socket);
    connect_socket = INVALID_SOCKET;
    WSACleanup ();
}

///////////////////////////////
//////////// UNIX /////////////
///////////////////////////////
#else

#include <netinet/in.h>
#include <netinet/tcp.h>


BroadCastClient::BroadCastClient (int port)
{
    this->port = port;
    connect_socket = -1;
    memset (&socket_addr, 0, sizeof (socket_addr));
}

int BroadCastClient::init ()
{
    connect_socket = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (connect_socket < 0)
    {
        return (int)BroadCastClientReturnCodes::CREATE_SOCKET_ERROR;
    }

    socket_addr.sin_family = AF_INET;
    socket_addr.sin_port = htons (port);
    socket_addr.sin_addr.s_addr = htonl (INADDR_ANY);

    // ensure that library will not hang in blocking recv/send call
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    setsockopt (connect_socket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof (tv));
    setsockopt (connect_socket, SOL_SOCKET, SO_SNDTIMEO, (const char *)&tv, sizeof (tv));
    int broadcast = 1;
    setsockopt (connect_socket, SOL_SOCKET, SO_BROADCAST, (char *)&broadcast, sizeof (broadcast));
    if (bind (connect_socket, (const struct sockaddr *)&socket_addr, sizeof (socket_addr)) != 0)
    {
        return (int)BroadCastClientReturnCodes::INIT_ERROR;
    }

    return (int)BroadCastClientReturnCodes::STATUS_OK;
}

int BroadCastClient::recv (void *data, int size)
{
    socklen_t len = (socklen_t)sizeof (socket_addr);
    int res = recvfrom (connect_socket, (char *)data, size, 0, (sockaddr *)&socket_addr, &len);
    return res;
}

void BroadCastClient::close ()
{
    ::close (connect_socket);
    connect_socket = -1;
}
#endif
