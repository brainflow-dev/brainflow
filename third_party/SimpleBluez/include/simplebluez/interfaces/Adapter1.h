#pragma once

#include <simpledbus/advanced/Interface.h>

#include <string>

namespace SimpleBluez {

class Adapter1 : public SimpleDBus::Interface {
  public:
    typedef enum { AUTO, BREDR, LE } DiscoveryFilter;

    Adapter1(std::shared_ptr<SimpleDBus::Connection> conn, std::string path);
    virtual ~Adapter1() = default;

    // ----- METHODS -----
    void StartDiscovery();
    void StopDiscovery();
    void SetDiscoveryFilter(DiscoveryFilter filter);
    SimpleDBus::Holder GetDiscoveryFilters();
    // void RemoveDevice(SimpleDBus::Holder device); // TODO: Implement

    // ----- PROPERTIES -----
    bool Discovering(bool refresh = true);
    std::string Address();

  protected:
    bool _discovering;
    std::string _address;

    void property_changed(std::string option_name) override;
};

}  // namespace SimpleBluez
