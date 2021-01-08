#include "socket_server_tcp.h"


///////////////////////////////
/////////// WINDOWS ///////////
//////////////////////////////
#ifdef _WIN32

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")

SocketServerTCP::SocketServerTCP (const char *local_ip, int local_port, bool recv_all_or_nothing)
{
    strcpy (this->local_ip, local_ip);
    this->local_port = local_port;
    this->recv_all_or_nothing = recv_all_or_nothing;
    server_socket = INVALID_SOCKET;
    connected_socket = INVALID_SOCKET;
    client_connected = false;
}

int SocketServerTCP::bind ()
{
    WSADATA wsadata;
    int res = WSAStartup (MAKEWORD (2, 2), &wsadata);
    if (res != 0)
    {
        return (int)SocketServerTCPReturnCodes::WSA_STARTUP_ERROR;
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons (local_port);
    if (inet_pton (AF_INET, local_ip, &server_addr.sin_addr) == 0)
    {
        return (int)SocketServerTCPReturnCodes::PTON_ERROR;
    }
    server_socket = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket == INVALID_SOCKET)
    {
        return (int)SocketServerTCPReturnCodes::CREATE_SOCKET_ERROR;
    }

    if (::bind (server_socket, (const struct sockaddr *)&server_addr, sizeof (server_addr)) != 0)
    {
        return (int)SocketServerTCPReturnCodes::CONNECT_ERROR;
    }

    // ensure that library will not hang in blocking recv/send call
    DWORD timeout = 3000;
    DWORD value = 1;
    DWORD buf_size = 65536 * 4;
    setsockopt (server_socket, IPPROTO_TCP, TCP_NODELAY, (const char *)&value, sizeof (value));
    setsockopt (server_socket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof (timeout));
    setsockopt (server_socket, SOL_SOCKET, SO_SNDTIMEO, (const char *)&timeout, sizeof (timeout));
    setsockopt (server_socket, SOL_SOCKET, SO_RCVBUF, (const char *)&buf_size, sizeof (buf_size));
    // set linger timeout to 1s to handle time_wait state gracefully
    struct linger sl;
    sl.l_onoff = 1;
    sl.l_linger = 1;
    setsockopt (server_socket, SOL_SOCKET, SO_LINGER, (const char *)&sl, sizeof (sl));

    if ((listen (server_socket, 1)) != 0)
    {
        return (int)SocketServerTCPReturnCodes::CONNECT_ERROR;
    }

    return (int)SocketServerTCPReturnCodes::STATUS_OK;
}

int SocketServerTCP::accept ()
{
    accept_thread = std::thread ([this] { this->accept_worker (); });
    return (int)SocketServerTCPReturnCodes::STATUS_OK;
}

void SocketServerTCP::accept_worker ()
{
    int len = sizeof (client_addr);
    connected_socket = ::accept (server_socket, (struct sockaddr *)&client_addr, &len);
    if (connected_socket != INVALID_SOCKET)
    {
        // ensure that library will not hang in blocking recv/send call
        DWORD timeout = 3000;
        DWORD value = 1;
        DWORD buf_size = 65536 * 4;
        setsockopt (
            connected_socket, IPPROTO_TCP, TCP_NODELAY, (const char *)&value, sizeof (value));
        setsockopt (
            connected_socket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof (timeout));
        setsockopt (
            connected_socket, SOL_SOCKET, SO_SNDTIMEO, (const char *)&timeout, sizeof (timeout));
        setsockopt (
            connected_socket, SOL_SOCKET, SO_RCVBUF, (const char *)&buf_size, sizeof (buf_size));

        client_connected = true;
    }
}

int SocketServerTCP::recv (void *data, int size)
{
    if (connected_socket == INVALID_SOCKET)
    {
        return -1;
    }
    int res = ::recv (connected_socket, (char *)data, size, 0);
    if (res == SOCKET_ERROR)
    {
        return -1;
    }
    for (int i = 0; i < res; i++)
    {
        temp_buffer.push (((char *)data)[i]);
    }
    // before we used SO_RCVLOWAT but it didnt work well
    // and we were not sure that it works correctly with timeout
    if (recv_all_or_nothing)
    {
        if ((int)temp_buffer.size () < size)
        {
            return 0;
        }
        for (int i = 0; i < size; i++)
        {
            ((char *)data)[i] = temp_buffer.front ();
            temp_buffer.pop ();
        }
        return size;
    }
    else
    {
        for (int i = 0; i < res; i++)
        {
            ((char *)data)[i] = temp_buffer.front ();
            temp_buffer.pop ();
        }
        return res;
    }
}

void SocketServerTCP::close ()
{
    if (server_socket != INVALID_SOCKET)
    {
        closesocket (server_socket);
        server_socket = INVALID_SOCKET;
    }
    if (accept_thread.joinable ())
    {
        accept_thread.join ();
    }
    if (connected_socket != INVALID_SOCKET)
    {
        closesocket (connected_socket);
        connected_socket = INVALID_SOCKET;
    }
    WSACleanup ();
}

///////////////////////////////
//////////// UNIX /////////////
///////////////////////////////
#else

#include <netinet/in.h>
#include <netinet/tcp.h>


SocketServerTCP::SocketServerTCP (const char *local_ip, int local_port, bool recv_all_or_nothing)
{
    strcpy (this->local_ip, local_ip);
    this->local_port = local_port;
    this->recv_all_or_nothing = recv_all_or_nothing;
    server_socket = -1;
    connected_socket = -1;
    client_connected = false;
}

int SocketServerTCP::bind ()
{
    server_socket = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket < 0)
    {
        return (int)SocketServerTCPReturnCodes::CREATE_SOCKET_ERROR;
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons (local_port);
    if (inet_pton (AF_INET, local_ip, &server_addr.sin_addr) == 0)
    {
        return (int)SocketServerTCPReturnCodes::PTON_ERROR;
    }

    if (::bind (server_socket, (const struct sockaddr *)&server_addr, sizeof (server_addr)) != 0)
    {
        return (int)SocketServerTCPReturnCodes::CONNECT_ERROR;
    }

    // ensure that library will not hang in blocking recv/send call
    struct timeval tv;
    tv.tv_sec = 3;
    tv.tv_usec = 0;
    int value = 1;
    int buf_size = 65536 * 4;
    setsockopt (server_socket, IPPROTO_TCP, TCP_NODELAY, &value, sizeof (value));
    setsockopt (server_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof (tv));
    setsockopt (server_socket, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof (tv));
    setsockopt (server_socket, SOL_SOCKET, SO_RCVBUF, &buf_size, sizeof (buf_size));
    // set linger timeout to 1s to handle time_wait state gracefully
    struct linger sl;
    sl.l_onoff = 1;
    sl.l_linger = 1;
    setsockopt (server_socket, SOL_SOCKET, SO_LINGER, &sl, sizeof (sl));

    if ((listen (server_socket, 1)) != 0)
    {
        return (int)SocketServerTCPReturnCodes::CONNECT_ERROR;
    }

    return (int)SocketServerTCPReturnCodes::STATUS_OK;
}

int SocketServerTCP::accept ()
{
    accept_thread = std::thread ([this] { this->accept_worker (); });
    return (int)SocketServerTCPReturnCodes::STATUS_OK;
}

void SocketServerTCP::accept_worker ()
{
    socklen_t len = (socklen_t)sizeof (client_addr);
    connected_socket = ::accept (server_socket, (struct sockaddr *)&this->client_addr, &len);
    if (connected_socket > 0)
    {
        // ensure that library will not hang in blocking recv/send call
        struct timeval tv;
        tv.tv_sec = 3;
        tv.tv_usec = 0;
        int value = 1;
        int buf_size = 65536 * 4;
        setsockopt (connected_socket, IPPROTO_TCP, TCP_NODELAY, &value, sizeof (value));
        setsockopt (connected_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof (tv));
        setsockopt (connected_socket, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof (tv));
        setsockopt (connected_socket, SOL_SOCKET, SO_RCVBUF, &buf_size, sizeof (buf_size));

        client_connected = true;
    }
}

int SocketServerTCP::recv (void *data, int size)
{
    if (connected_socket <= 0)
    {
        return -1;
    }
    int res = ::recv (connected_socket, (char *)data, size, 0);
    if (res < 0)
    {
        return res;
    }
    for (int i = 0; i < res; i++)
    {
        temp_buffer.push (((char *)data)[i]);
    }
    // before we used SO_RCVLOWAT but it didnt work well
    // and we were not sure that it works correctly with timeout
    if (recv_all_or_nothing)
    {
        if (temp_buffer.size () < size)
        {
            return 0;
        }
        for (int i = 0; i < size; i++)
        {
            ((char *)data)[i] = temp_buffer.front ();
            temp_buffer.pop ();
        }
        return size;
    }
    else
    {
        for (int i = 0; i < res; i++)
        {
            ((char *)data)[i] = temp_buffer.front ();
            temp_buffer.pop ();
        }
        return res;
    }
}

void SocketServerTCP::close ()
{
    if (server_socket != -1)
    {
        ::close (server_socket);
        server_socket = -1;
    }
    if (accept_thread.joinable ())
    {
        accept_thread.join ();
    }
    if (connected_socket != -1)
    {
        ::close (connected_socket);
        connected_socket = -1;
    }
}
#endif
