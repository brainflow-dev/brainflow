#pragma once

#include <simpledbus/advanced/Interface.h>
#include <simpledbus/external/kvn_safe_callback.hpp>

#include <string>

namespace SimpleBluez {

class Device1 : public SimpleDBus::Interface {
  public:
    Device1(std::shared_ptr<SimpleDBus::Connection> conn, std::string path);
    virtual ~Device1();

    // ----- METHODS -----
    void Connect();
    void Disconnect();
    void Pair();
    void CancelPairing();

    // ----- PROPERTIES -----
    int16_t RSSI();
    uint16_t Appearance();  // On Bluez 5.53, this always returns 0.
    std::string Address();
    std::string Alias();
    std::string Name();
    std::map<uint16_t, std::vector<uint8_t>> ManufacturerData(bool refresh = true);
    bool Paired(bool refresh = true);
    bool Connected(bool refresh = true);
    bool ServicesResolved(bool refresh = true);

    // ----- CALLBACKS -----
    kvn::safe_callback<void()> OnServicesResolved;
    kvn::safe_callback<void()> OnDisconnected;

  protected:
    void property_changed(std::string option_name) override;

    int16_t _rssi;
    std::string _name;
    std::string _alias;
    std::string _address;
    bool _connected;
    bool _services_resolved;
    std::map<uint16_t, std::vector<uint8_t>> _manufacturer_data;
};

}  // namespace SimpleBluez
