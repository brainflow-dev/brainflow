#pragma once

#include "simpledbus/SimpleDBus.h"

#include <string>

class Adapter1 : public SimpleDBus::Interfaces::PropertyHandler, public SimpleDBus::Properties {
  private:
    static const std::string _interface_name;

    SimpleDBus::Connection* _conn;
    std::string _path;

    bool _discovering;
    std::string _address;

    void add_option(std::string option_name, SimpleDBus::Holder value);
    void remove_option(std::string option_name);

  public:
    Adapter1(SimpleDBus::Connection* conn, std::string path);
    ~Adapter1();

    // DBus Methods
    void StartDiscovery();
    void StopDiscovery();
    void SetDiscoveryFilter(SimpleDBus::Holder properties);
    std::string Address();
    SimpleDBus::Holder GetDiscoveryFilters();

    void Action_StartDiscovery();
    void Action_StopDiscovery();

    bool Property_Discovering();

    bool is_discovering();

    std::function<void(void)> OnDiscoveryStarted;
    std::function<void(void)> OnDiscoveryStopped;
};
