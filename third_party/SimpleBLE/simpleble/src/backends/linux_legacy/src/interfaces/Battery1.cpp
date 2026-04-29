#include "simplebluezlegacy/interfaces/Battery1.h"

#include <iostream>

using namespace SimpleBluezLegacy;

const SimpleDBusLegacy::AutoRegisterInterface<Battery1> Battery1::registry{
    "org.bluez.Battery1",
    // clang-format off
    [](std::shared_ptr<SimpleDBusLegacy::Connection> conn, const std::string& bus_name, const std::string& path, const SimpleDBusLegacy::Holder& options) -> std::shared_ptr<SimpleDBusLegacy::Interface> {
        return std::static_pointer_cast<SimpleDBusLegacy::Interface>(std::make_shared<Battery1>(conn, path));
    }
    // clang-format on
};

Battery1::Battery1(std::shared_ptr<SimpleDBusLegacy::Connection> conn, std::string path)
    : SimpleDBusLegacy::Interface(conn, "org.bluez", path, "org.bluez.Battery1") {}

Battery1::~Battery1() { OnPercentageChanged.unload(); }

uint8_t Battery1::Percentage() {
    std::scoped_lock lock(_property_update_mutex);
    return _properties["Percentage"].get_byte();
}

void Battery1::property_changed(std::string option_name) {
    if (option_name == "Percentage") {
        OnPercentageChanged();
    }
}
