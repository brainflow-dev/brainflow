#pragma once

#include "simpledbus/SimpleDBus.h"

#include <string>
#include <map>
#include <vector>

class Device1 : public SimpleDBus::Interfaces::PropertyHandler, public SimpleDBus::Properties {
  private:
    static const std::string _interface_name;

    SimpleDBus::Connection* _conn;
    std::string _path;

    int16_t _rssi;
    std::string _name;
    std::string _alias;
    std::string _address;
    bool _services_resolved;
    std::map<uint16_t, std::vector<uint8_t>> _manufacturer_data;

    void add_option(std::string option_name, SimpleDBus::Holder value);
    void remove_option(std::string option_name);

  public:
    Device1(SimpleDBus::Connection* conn, std::string path);
    ~Device1();

    void Connect();
    void Disconnect();

    int16_t get_rssi();
    std::string get_name();
    std::string get_alias();
    std::string get_address();
    std::map<uint16_t, std::vector<uint8_t>> get_manufacturer_data();
    bool is_connected();
    bool is_services_resolved();

    void Action_Connect();
    void Action_Disconnect();

    bool Property_Connected();
    bool Property_ServicesResolved();

    std::function<void(void)> OnConnected;
    std::function<void(void)> OnDisconnected;
    std::function<void(void)> OnServicesResolved;
};
