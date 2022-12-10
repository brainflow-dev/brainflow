#pragma once

#include <simpledbus/advanced/Interface.h>

#include <optional>
#include <string>
#include <vector>

namespace SimpleBluez {

class Adapter1 : public SimpleDBus::Interface {
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
    Adapter1(std::shared_ptr<SimpleDBus::Connection> conn, std::string path);
    virtual ~Adapter1() = default;

    // ----- METHODS -----
    void RemoveDevice(std::string device_path);
    void StartDiscovery();
    void StopDiscovery();
    void SetDiscoveryFilter(DiscoveryFilter filter);
    SimpleDBus::Holder GetDiscoveryFilters();

    // ----- PROPERTIES -----
    bool Discovering(bool refresh = true);
    bool Powered(bool refresh = true);
    std::string Address();

  protected:
    void property_changed(std::string option_name) override;
};

}  // namespace SimpleBluez
