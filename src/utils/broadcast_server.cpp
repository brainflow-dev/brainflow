#include <string.h>

#include "broadcast_server.h"


///////////////////////////////
/////////// WINDOWS ///////////
//////////////////////////////
#ifdef _WIN32

#ifndef INET_ADDRSTRLEN
#define INET_ADDRSTRLEN 46
#endif

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")


BroadCastServer::BroadCastServer (int port)
{
    this->port = port;
    connect_socket = INVALID_SOCKET;
    memset (&socket_addr, 0, sizeof (socket_addr));
    strcpy (address, "255.255.255.255");
    wsa_initialized = false;
}

BroadCastServer::BroadCastServer (const char *address, int port)
{
    this->port = port;
    connect_socket = INVALID_SOCKET;
    memset (&socket_addr, 0, sizeof (socket_addr));
    strcpy (this->address, address);
    wsa_initialized = false;
}

int BroadCastServer::init ()
{
    WSADATA wsadata;
    if (wsa_initialized)
    {
        return (int)BroadCastServerReturnCodes::SOCKET_ALREADY_CREATED_ERROR;
    }
    int res = WSAStartup (MAKEWORD (2, 2), &wsadata);
    if (res != 0)
    {
        return (int)BroadCastServerReturnCodes::WSA_STARTUP_ERROR;
    }
    wsa_initialized = true;
    socket_addr.sin_family = AF_INET;
    socket_addr.sin_port = htons (port);
    if (inet_pton (AF_INET, address, &socket_addr.sin_addr) == 0)
    {
        return (int)BroadCastServerReturnCodes::INIT_ERROR;
    }
    connect_socket = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (connect_socket == INVALID_SOCKET)
    {
        return (int)BroadCastServerReturnCodes::CREATE_SOCKET_ERROR;
    }

    // ensure that library will not hang in blocking recv/send call
    DWORD timeout = 5000;
    setsockopt (connect_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof (timeout));
    setsockopt (connect_socket, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof (timeout));
    DWORD broadcast = 1;
    setsockopt (connect_socket, SOL_SOCKET, SO_BROADCAST, (char *)&broadcast, sizeof (broadcast));

    return (int)BroadCastServerReturnCodes::STATUS_OK;
}

int BroadCastServer::send (const char *data, int size)
{
    int len = sizeof (socket_addr);
    int res = sendto (connect_socket, data, size, 0, (sockaddr *)&socket_addr, len);
    if (res == SOCKET_ERROR)
    {
        return -1;
    }
    return res;
}

int BroadCastServer::recv (void *data, int size, char *sender_ip, int max_len)
{
    char ip_address_sender[INET_ADDRSTRLEN];
    memset (ip_address_sender, 0, sizeof (char) * INET_ADDRSTRLEN);
    struct sockaddr_in socket_addr_recv;
    memset (&socket_addr_recv, 0, sizeof (socket_addr_recv));
    int len = sizeof (socket_addr_recv);
    int res = recvfrom (connect_socket, (char *)data, size, 0, (sockaddr *)&socket_addr_recv, &len);
    if (res == SOCKET_ERROR)
    {
        return -1;
    }
    if (inet_ntop (AF_INET, &(socket_addr_recv.sin_addr), ip_address_sender, INET_ADDRSTRLEN) ==
        NULL)
    {
        return -2;
    }
    strncpy (sender_ip, ip_address_sender, max_len);
    return res;
}

void BroadCastServer::close ()
{
    closesocket (connect_socket);
    connect_socket = INVALID_SOCKET;
    if (wsa_initialized)
    {
        WSACleanup ();
        wsa_initialized = false;
    }
}

///////////////////////////////
//////////// UNIX /////////////
///////////////////////////////
#else

#include <netinet/in.h>
#include <netinet/tcp.h>

#ifndef INET_ADDRSTRLEN
#define INET_ADDRSTRLEN 46
#endif


BroadCastServer::BroadCastServer (int port)
{
    this->port = port;
    connect_socket = -1;
    memset (&socket_addr, 0, sizeof (socket_addr));
    strcpy (address, "255.255.255.255");
}

BroadCastServer::BroadCastServer (const char *address, int port)
{
    this->port = port;
    connect_socket = -1;
    memset (&socket_addr, 0, sizeof (socket_addr));
    strcpy (this->address, address);
}

int BroadCastServer::init ()
{
    connect_socket = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (connect_socket < 0)
    {
        return (int)BroadCastServerReturnCodes::CREATE_SOCKET_ERROR;
    }

    socket_addr.sin_family = AF_INET;
    socket_addr.sin_port = htons (port);
    socket_addr.sin_addr.s_addr = htonl (INADDR_ANY);
    if (inet_pton (AF_INET, address, &socket_addr.sin_addr) == 0)
    {
        return (int)BroadCastServerReturnCodes::INIT_ERROR;
    }

    // ensure that library will not hang in blocking recv/send call
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    setsockopt (connect_socket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof (tv));
    setsockopt (connect_socket, SOL_SOCKET, SO_SNDTIMEO, (const char *)&tv, sizeof (tv));
    int broadcast = 1;
    setsockopt (connect_socket, SOL_SOCKET, SO_BROADCAST, (char *)&broadcast, sizeof (broadcast));
    return (int)BroadCastServerReturnCodes::STATUS_OK;
}

int BroadCastServer::send (const char *data, int size)
{
    socklen_t len = (socklen_t)sizeof (socket_addr);
    int res = sendto (connect_socket, data, size, 0, (sockaddr *)&socket_addr, len);
    return res;
}

int BroadCastServer::recv (void *data, int size, char *sender_ip, int max_len)
{
    char ip_address_sender[INET_ADDRSTRLEN];
    memset (ip_address_sender, 0, sizeof (char) * INET_ADDRSTRLEN);
    struct sockaddr_in socket_addr_recv;
    memset (&socket_addr_recv, 0, sizeof (socket_addr_recv));
    socklen_t len = (socklen_t)sizeof (socket_addr_recv);
    int res = recvfrom (connect_socket, (char *)data, size, 0, (sockaddr *)&socket_addr_recv, &len);
    if (res > 0)
    {
        if (inet_ntop (AF_INET, &(socket_addr_recv.sin_addr), ip_address_sender, INET_ADDRSTRLEN) ==
            NULL)
        {
            return -2;
        }
    }
    strncpy (sender_ip, ip_address_sender, max_len);
    return res;
}

void BroadCastServer::close ()
{
    ::close (connect_socket);
    connect_socket = -1;
}
#endif
