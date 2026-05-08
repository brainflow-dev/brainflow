#pragma once

#include <simpledbuslegacy/advanced/Interface.h>
#include <simpledbuslegacy/advanced/InterfaceRegistry.h>

#include <optional>
#include <string>
#include <vector>

namespace SimpleBluezLegacy {

class Adapter1 : public SimpleDBusLegacy::Interface {
  public:
    // ----- TYPES -----
    struct DiscoveryFilter {
        typedef enum { AUTO, BREDR, LE } TransportType;
        std::vector<std::string> UUIDs = {};
        std::optional<int16_t> RSSI;
        std::optional<uint16_t> Pathloss;
        TransportType Transport = TransportType::AUTO;
        bool DuplicateData = true;
        bool Discoverable = false;
        std::string Pattern = "";
    };

    // ----- CONSTRUCTORS -----
    Adapter1(std::shared_ptr<SimpleDBusLegacy::Connection> conn, std::string path);
    virtual ~Adapter1() = default;

    // ----- METHODS -----
    void RemoveDevice(std::string device_path);
    void StartDiscovery();
    void StopDiscovery();
    void SetDiscoveryFilter(DiscoveryFilter filter);
    SimpleDBusLegacy::Holder GetDiscoveryFilters();

    // ----- PROPERTIES -----
    bool Discovering(bool refresh = true);
    bool Powered(bool refresh = true);
    std::string Address();

  protected:
    void property_changed(std::string option_name) override;

  private:
    static const SimpleDBusLegacy::AutoRegisterInterface<Adapter1> registry;
};

}  // namespace SimpleBluezLegacy
