#include <simplebluez/Device.h>
#include <simplebluez/Exceptions.h>
#include <simplebluez/Service.h>

using namespace SimpleBluez;

Device::Device(std::shared_ptr<SimpleDBus::Connection> conn, const std::string& bus_name, const std::string& path)
    : Proxy(conn, bus_name, path) {}

Device::~Device() {}

std::shared_ptr<SimpleDBus::Proxy> Device::path_create(const std::string& path) {
    auto child = std::make_shared<Service>(_conn, _bus_name, path);
    return std::static_pointer_cast<SimpleDBus::Proxy>(child);
}

std::shared_ptr<SimpleDBus::Interface> Device::interfaces_create(const std::string& interface_name) {
    if (interface_name == "org.bluez.Device1") {
        return std::static_pointer_cast<SimpleDBus::Interface>(std::make_shared<Device1>(_conn, _path));
    } else if (interface_name == "org.bluez.Battery1") {
        return std::static_pointer_cast<SimpleDBus::Interface>(std::make_shared<Battery1>(_conn, _path));
    }

    auto interface = std::make_shared<SimpleDBus::Interface>(_conn, _bus_name, _path, interface_name);
    return std::static_pointer_cast<SimpleDBus::Interface>(interface);
}

std::shared_ptr<Device1> Device::device1() {
    return std::dynamic_pointer_cast<Device1>(interface_get("org.bluez.Device1"));
}

std::shared_ptr<Battery1> Device::battery1() {
    return std::dynamic_pointer_cast<Battery1>(interface_get("org.bluez.Battery1"));
}

std::vector<std::shared_ptr<Service>> Device::services() { return children_casted<Service>(); }

std::shared_ptr<Service> Device::get_service(const std::string& uuid) {
    auto services_all = services();

    for (auto& service : services_all) {
        if (service->uuid() == uuid) {
            return service;
        }
    }

    throw Exception::ServiceNotFoundException(uuid);
}

std::shared_ptr<Characteristic> Device::get_characteristic(const std::string& service_uuid,
                                                           const std::string& characteristic_uuid) {
    auto service = get_service(service_uuid);
    return service->get_characteristic(characteristic_uuid);
}

void Device::pair() { device1()->Pair(); }

void Device::cancel_pairing() { device1()->CancelPairing(); }

void Device::connect() { device1()->Connect(); }

void Device::disconnect() { device1()->Disconnect(); }

std::string Device::address() { return device1()->Address(); }

std::string Device::address_type() { return device1()->AddressType(); }

std::string Device::name() { return device1()->Name(); }

std::string Device::alias() { return device1()->Alias(); }

int16_t Device::rssi() { return device1()->RSSI(); }

int16_t Device::tx_power() { return device1()->TxPower(); }

std::map<uint16_t, std::vector<uint8_t>> Device::manufacturer_data() { return device1()->ManufacturerData(); }

std::map<std::string, std::vector<uint8_t>> Device::service_data() { return device1()->ServiceData(); }

bool Device::paired() { return device1()->Paired(); }

bool Device::connected() { return device1()->Connected(); }

bool Device::services_resolved() { return device1()->ServicesResolved(); }

void Device::set_on_disconnected(std::function<void()> callback) { device1()->OnDisconnected.load(callback); }

void Device::clear_on_disconnected() { device1()->OnDisconnected.unload(); }

void Device::set_on_services_resolved(std::function<void()> callback) { device1()->OnServicesResolved.load(callback); }

void Device::clear_on_services_resolved() { device1()->OnServicesResolved.unload(); }

bool Device::has_battery_interface() { return interface_exists("org.bluez.Battery1"); }

uint8_t Device::battery_percentage() { return battery1()->Percentage(); }

void Device::set_on_battery_percentage_changed(std::function<void(uint8_t new_value)> callback) {
    battery1()->OnPercentageChanged.load([this, callback]() { callback(battery1()->Percentage()); });
    // As the `property_changed` callback only occurs when the property is changed, we need to manually
    // call the callback once to make sure the callback is called with the current value.
    battery1()->OnPercentageChanged();
}

void Device::clear_on_battery_percentage_changed() { battery1()->OnPercentageChanged.unload(); }
