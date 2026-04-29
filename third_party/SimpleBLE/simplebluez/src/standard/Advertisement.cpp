#include <simplebluez/standard/Advertisement.h>

using namespace SimpleBluez;

Advertisement::Advertisement(std::shared_ptr<SimpleDBus::Connection> conn, const std::string& bus_name,
                             const std::string& path)
    : Proxy(conn, bus_name, path) {}

void Advertisement::on_registration() {
    _interfaces.emplace(std::make_pair("org.bluez.LEAdvertisement1",
                                       std::static_pointer_cast<SimpleDBus::Interface>(
                                           std::make_shared<LEAdvertisement1>(_conn, shared_from_this()))));

    _interfaces.emplace(
        std::make_pair("org.freedesktop.DBus.ObjectManager",
                       std::static_pointer_cast<SimpleDBus::Interface>(
                           std::make_shared<SimpleDBus::Interfaces::ObjectManager>(_conn, shared_from_this()))));

    le_advertisement1()->OnRelease.load([this]() { _active.store(false); });
}

std::shared_ptr<LEAdvertisement1> Advertisement::le_advertisement1() {
    return std::dynamic_pointer_cast<LEAdvertisement1>(interface_get("org.bluez.LEAdvertisement1"));
}

std::shared_ptr<SimpleDBus::Interfaces::ObjectManager> Advertisement::object_manager() {
    return std::dynamic_pointer_cast<SimpleDBus::Interfaces::ObjectManager>(
        interface_get("org.freedesktop.DBus.ObjectManager"));
}

bool Advertisement::active() { return _active.load(); }

void Advertisement::activate() { _active.store(true); }

void Advertisement::deactivate() { _active.store(false); }

std::string Advertisement::adv_type() { return le_advertisement1()->Type(); }

void Advertisement::adv_type(const std::string& type) { le_advertisement1()->Type(type); }

std::vector<std::string> Advertisement::service_uuids() { return le_advertisement1()->ServiceUUIDs(); }

void Advertisement::service_uuids(const std::vector<std::string>& service_uuids) {
    le_advertisement1()->ServiceUUIDs(service_uuids);
}

std::map<uint16_t, ByteArray> Advertisement::manufacturer_data() { return le_advertisement1()->ManufacturerData(); }

void Advertisement::manufacturer_data(const std::map<uint16_t, ByteArray>& manufacturer_data) {
    le_advertisement1()->ManufacturerData(manufacturer_data);
}

std::map<std::string, ByteArray> Advertisement::service_data() { return le_advertisement1()->ServiceData(); }

void Advertisement::service_data(const std::map<std::string, ByteArray>& service_data) {
    le_advertisement1()->ServiceData(service_data);
}

std::vector<std::string> Advertisement::solicit_uuids() { return le_advertisement1()->SolicitUUIDs(); }

void Advertisement::solicit_uuids(const std::vector<std::string>& solicit_uuids) {
    le_advertisement1()->SolicitUUIDs(solicit_uuids);
}

std::map<uint8_t, ByteArray> Advertisement::data() { return le_advertisement1()->Data(); }

void Advertisement::data(const std::map<uint8_t, ByteArray>& data) { le_advertisement1()->Data(data); }

bool Advertisement::discoverable() { return le_advertisement1()->Discoverable(); }

void Advertisement::discoverable(bool discoverable) { le_advertisement1()->Discoverable(discoverable); }

uint16_t Advertisement::discoverable_timeout() { return le_advertisement1()->DiscoverableTimeout(); }

void Advertisement::discoverable_timeout(uint16_t timeout) { le_advertisement1()->DiscoverableTimeout(timeout); }

std::vector<std::string> Advertisement::includes() { return le_advertisement1()->Includes(); }

void Advertisement::includes(const std::vector<std::string>& includes) { le_advertisement1()->Includes(includes); }

std::string Advertisement::local_name() { return le_advertisement1()->LocalName(); }

void Advertisement::local_name(const std::string& name) { le_advertisement1()->LocalName(name); }

uint16_t Advertisement::appearance() { return le_advertisement1()->Appearance(); }

void Advertisement::appearance(uint16_t appearance) { le_advertisement1()->Appearance(appearance); }

uint16_t Advertisement::duration() { return le_advertisement1()->Duration(); }

void Advertisement::duration(uint16_t duration) { le_advertisement1()->Duration(duration); }

uint16_t Advertisement::timeout() { return le_advertisement1()->Timeout(); }

void Advertisement::timeout(uint16_t timeout) { le_advertisement1()->Timeout(timeout); }

uint32_t Advertisement::min_interval() { return le_advertisement1()->MinInterval(); }

void Advertisement::min_interval(uint32_t interval) { le_advertisement1()->MinInterval(interval); }

uint32_t Advertisement::max_interval() { return le_advertisement1()->MaxInterval(); }

void Advertisement::max_interval(uint32_t interval) { le_advertisement1()->MaxInterval(interval); }

int16_t Advertisement::tx_power() { return le_advertisement1()->TxPower(); }

void Advertisement::tx_power(int16_t power) { le_advertisement1()->TxPower(power); }

bool Advertisement::include_tx_power() { return le_advertisement1()->IncludeTxPower(); }

void Advertisement::include_tx_power(bool include) { le_advertisement1()->IncludeTxPower(include); }