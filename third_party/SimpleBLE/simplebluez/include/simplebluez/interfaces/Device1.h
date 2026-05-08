#pragma once

#include <simpledbus/advanced/Interface.h>
#include <simpledbus/advanced/InterfaceRegistry.h>

#include <string>

#include "simplebluez/Types.h"

namespace SimpleBluez {

class Device1 : public SimpleDBus::Interface {
  public:
    Device1(std::shared_ptr<SimpleDBus::Connection> conn, std::shared_ptr<SimpleDBus::Proxy> proxy);
    virtual ~Device1();

    // ----- METHODS -----
    void Connect();
    void Disconnect();
    void Pair();
    void CancelPairing();

    // ----- PROPERTIES -----
    Property<int16_t>& RSSI = property<int16_t>("RSSI");
    Property<int16_t>& TxPower = property<int16_t>("TxPower");
    Property<uint16_t>& Appearance = property<uint16_t>("Appearance");
    Property<std::string>& Address = property<std::string>("Address");
    Property<std::string>& AddressType = property<std::string>("AddressType");
    Property<std::string>& Alias = property<std::string>("Alias");
    Property<std::string>& Name = property<std::string>("Name");
    Property<std::vector<std::string>>& UUIDs = property<std::vector<std::string>>("UUIDs");
    Property<std::map<uint16_t, ByteArray>>& ManufacturerData = property<std::map<uint16_t, ByteArray>>("ManufacturerData");
    Property<std::map<std::string, ByteArray>>& ServiceData = property<std::map<std::string, ByteArray>>("ServiceData");
    Property<bool>& Paired = property<bool>("Paired");
    Property<bool>& Bonded = property<bool>("Bonded");
    Property<bool>& Trusted = property<bool>("Trusted");
    Property<bool>& Connected = property<bool>("Connected");
    Property<bool>& ServicesResolved = property<bool>("ServicesResolved");

  private:
    static const SimpleDBus::AutoRegisterInterface<Device1> registry;
};

}  // namespace SimpleBluez
