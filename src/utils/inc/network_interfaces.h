#pragma once

#include <string>
#include <vector>

#ifdef _WIN32

#include <iphlpapi.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

static std::vector<std::string> get_broadcast_addresses ()
{
    std::vector<std::string> result;
    int i = 0;
    PMIB_IPADDRTABLE p_ip_addr_table;
    DWORD dw_size = 0;
    DWORD dw_ret_val = 0;
    IN_ADDR ip_addr;

    p_ip_addr_table = (MIB_IPADDRTABLE *)malloc (sizeof (MIB_IPADDRTABLE));
    if (p_ip_addr_table)
    {
        // Make an initial call to GetIpAddrTable to get the
        // necessary size into the dw_size variable
        if (GetIpAddrTable (p_ip_addr_table, &dw_size, 0) == ERROR_INSUFFICIENT_BUFFER)
        {
            free (p_ip_addr_table);
            p_ip_addr_table = (MIB_IPADDRTABLE *)malloc (dw_size);
        }
        if (p_ip_addr_table == NULL)
        {
            return result;
        }
    }
    // Make a second call to GetIpAddrTable to get the
    // actual data we want
    if ((dw_ret_val = GetIpAddrTable (p_ip_addr_table, &dw_size, 0)) != NO_ERROR)
    {
        return result;
    }

    for (i = 0; i < (int)p_ip_addr_table->dwNumEntries; i++)
    {
        ip_addr.S_un.S_addr = p_ip_addr_table->table[i].dwAddr | ~p_ip_addr_table->table[i].dwMask;
        char bw_addr[32];
        if (inet_ntop (AF_INET, &ip_addr, bw_addr, 32) != NULL)
        {
            result.push_back (bw_addr);
        }
    }

    if (p_ip_addr_table)
    {
        free (p_ip_addr_table);
        p_ip_addr_table = NULL;
    }
    return result;
}

#else

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <sys/socket.h>

#include <algorithm>

static std::vector<std::string> get_broadcast_addresses ()
{
    std::vector<std::string> result;
    struct ifaddrs *ifap = NULL;
    struct ifaddrs *ifa = NULL;
    char bw_addr[32];
    struct in_addr ip_addr;
    ip_addr.s_addr = 0;

    if (getifaddrs (&ifap) == 0)
    {
        for (ifa = ifap; ifa; ifa = ifa->ifa_next)
        {
            if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET)
            {
                ip_addr.s_addr = ((struct sockaddr_in *)ifa->ifa_addr)->sin_addr.s_addr |
                    ~(((struct sockaddr_in *)ifa->ifa_netmask)->sin_addr.s_addr);
                if (inet_ntop (AF_INET, &ip_addr, bw_addr, 32) != NULL)
                {
                    // somehow it creates duplicates
                    std::string bw_str = bw_addr;
                    if (std::find (result.begin (), result.end (), bw_str) == result.end ())
                    {
                        result.push_back (bw_str);
                    }
                }
            }
        }

        freeifaddrs (ifap);
    }

    return result;
}

#endif
