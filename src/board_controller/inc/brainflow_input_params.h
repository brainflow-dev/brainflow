#pragma once

#include <string>
#include <tuple>

enum class IpProtocolType
{
    NONE = 0,
    UDP = 1,
    TCP = 2
};

// we pass this structure from user API as a json string
struct BrainFlowInputParams
{
    std::string serial_port;
    std::string mac_address;
    std::string ip_address;
    int ip_port;
    int ip_protocol;
    std::string other_info;
    int timeout;
    std::string serial_number;

    BrainFlowInputParams ()
    {
        serial_port = "";
        mac_address = "";
        ip_address = "";
        ip_port = 0;
        ip_protocol = (int)IpProtocolType::NONE;
        other_info = "";
        timeout = 0;
        serial_number = "";
    }

    // default copy constructor and assignment operator are ok, need less operator to use in map
    bool operator< (const struct BrainFlowInputParams &other) const
    {
        return std::tie (serial_port, mac_address, ip_address, ip_port, ip_protocol, other_info,
                   timeout, serial_number) < std::tie (other.serial_port, other.mac_address,
                                                 other.ip_address, other.ip_port, other.ip_protocol,
                                                 other.other_info, timeout, serial_number);
    }
};