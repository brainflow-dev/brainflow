#pragma once
#include <simpledbus/advanced/Interface.h>
#include <simpledbus/advanced/InterfaceRegistry.h>

#include <string>
#include <vector>

namespace SimpleBluez {

class LEAdvertisingManager1 : public SimpleDBus::Interface {
  public:
    // ----- TYPES -----

    // ----- CONSTRUCTORS -----

    LEAdvertisingManager1(std::shared_ptr<SimpleDBus::Connection> conn, std::shared_ptr<SimpleDBus::Proxy> proxy);
    virtual ~LEAdvertisingManager1();

    // ----- METHODS -----

    void RegisterAdvertisement(std::string advertisement_path);
    void UnregisterAdvertisement(std::string advertisement_path);

    // ----- PROPERTIES -----

    Property<uint8_t>& ActiveInstances = property<uint8_t>("ActiveInstances");
    Property<uint8_t>& SupportedInstances = property<uint8_t>("SupportedInstances");
    Property<std::vector<std::string>>& SupportedIncludes = property<std::vector<std::string>>("SupportedIncludes");

  private:
    static const SimpleDBus::AutoRegisterInterface<LEAdvertisingManager1> registry;
};

}  // namespace SimpleBluez