#pragma once


#include "multicast_client.h"


class MultiCastServer
{

public:
    MultiCastServer (const char *local_ip, int local_port);
    ~MultiCastServer ()
    {
        close ();
    }

    int init ();
    int send (void *data, int size);
    void close ();

private:
    char local_ip[80];
    int local_port;
    struct sockaddr_in server_addr;

#ifdef _WIN32
    volatile SOCKET server_socket;
#else
    volatile int server_socket;
#endif
};
