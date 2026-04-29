#pragma once
#include <simpledbus/advanced/Interface.h>
#include <simpledbus/advanced/InterfaceRegistry.h>

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "simplebluez/Types.h"

namespace SimpleBluez {

class LEAdvertisement1 : public SimpleDBus::Interface {
  public:
    // ----- CONSTRUCTORS -----

    LEAdvertisement1(std::shared_ptr<SimpleDBus::Connection> conn, std::shared_ptr<SimpleDBus::Proxy> proxy);
    virtual ~LEAdvertisement1();

    // ----- SIGNALS -----
    kvn::safe_callback<void()> OnRelease;

    // ----- PROPERTIES -----

    Property<std::string>& Type = property<std::string>("Type", "peripheral");
    Property<std::vector<std::string>>& ServiceUUIDs = property<std::vector<std::string>>("ServiceUUIDs");
    Property<std::map<uint16_t, ByteArray>>& ManufacturerData = property<std::map<uint16_t, ByteArray>>("ManufacturerData");
    Property<std::map<std::string, ByteArray>>& ServiceData = property<std::map<std::string, ByteArray>>("ServiceData");
    Property<std::vector<std::string>>& SolicitUUIDs = property<std::vector<std::string>>("SolicitUUIDs");
    Property<std::map<uint8_t, ByteArray>>& Data = property<std::map<uint8_t, ByteArray>>("Data");
    Property<bool>& Discoverable = property<bool>("Discoverable");
    Property<uint16_t>& DiscoverableTimeout = property<uint16_t>("DiscoverableTimeout");
    Property<std::vector<std::string>>& Includes = property<std::vector<std::string>>("Includes");
    Property<std::string>& LocalName = property<std::string>("LocalName");
    Property<uint16_t>& Appearance = property<uint16_t>("Appearance");
    Property<uint16_t>& Duration = property<uint16_t>("Duration");
    Property<uint16_t>& Timeout = property<uint16_t>("Timeout");
    // Property<std::string>& SecondaryChannel = property<std::string>("SecondaryChannel");
    Property<uint32_t>& MinInterval = property<uint32_t>("MinInterval");
    Property<uint32_t>& MaxInterval = property<uint32_t>("MaxInterval");
    Property<int16_t>& TxPower = property<int16_t>("TxPower");
    Property<bool>& IncludeTxPower = property<bool>("IncludeTxPower");

    void message_handle(SimpleDBus::Message& msg) override;

  private:
    static const SimpleDBus::AutoRegisterInterface<LEAdvertisement1> registry;
};

}  // namespace SimpleBluez