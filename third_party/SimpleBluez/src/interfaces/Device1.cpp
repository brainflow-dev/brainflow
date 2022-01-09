#include "simplebluez/interfaces/Device1.h"

using namespace SimpleBluez;

Device1::Device1(std::shared_ptr<SimpleDBus::Connection> conn, std::string path)
    : SimpleDBus::Interface(conn, "org.bluez", path, "org.bluez.Device1") {}

Device1::~Device1() {
    OnDisconnected.unload();
    OnServicesResolved.unload();
}

void Device1::Connect() {
    auto msg = create_method_call("Connect");
    _conn->send_with_reply_and_block(msg);
}

void Device1::Disconnect() {
    auto msg = create_method_call("Disconnect");
    _conn->send_with_reply_and_block(msg);
}

int16_t Device1::RSSI() {
    std::scoped_lock lock(_property_update_mutex);
    return _properties["RSSI"].get_int16();
}

uint16_t Device1::Appearance() {
    std::scoped_lock lock(_property_update_mutex);
    return _properties["Appearance"].get_uint16();
}

std::string Device1::Address() {
    std::scoped_lock lock(_property_update_mutex);
    return _properties["Address"].get_string();
}

std::string Device1::Alias() {
    std::scoped_lock lock(_property_update_mutex);
    return _properties["Alias"].get_string();
}

std::string Device1::Name() {
    std::scoped_lock lock(_property_update_mutex);
    return _properties["Name"].get_string();
}

std::map<uint16_t, std::vector<uint8_t>> Device1::ManufacturerData(bool refresh) {
    if (refresh) {
        property_refresh("ManufacturerData");
    }

    // Use the locally cached version to avoid parsing the map multiple times.
    std::scoped_lock lock(_property_update_mutex);
    return _manufacturer_data;
}

bool Device1::Connected(bool refresh) {
    if (refresh) {
        property_refresh("Connected");
    }

    std::scoped_lock lock(_property_update_mutex);
    return _properties["Connected"].get_boolean();
}

bool Device1::ServicesResolved(bool refresh) {
    if (refresh) {
        property_refresh("ServicesResolved");
    }

    std::scoped_lock lock(_property_update_mutex);
    return _properties["ServicesResolved"].get_boolean();
}

void Device1::property_changed(std::string option_name) {
    if (option_name == "Connected") {
        if (!Connected(false)) {
            OnDisconnected();
        }
    } else if (option_name == "ServicesResolved") {
        if (ServicesResolved(false)) {
            OnServicesResolved();
        }
    } else if (option_name == "ManufacturerData") {
        std::scoped_lock lock(_property_update_mutex);

        _manufacturer_data.clear();
        std::map<uint16_t, SimpleDBus::Holder> manuf_data = _properties["ManufacturerData"].get_dict_uint16();
        // Loop through all received keys and store them.
        for (auto& [key, value_array] : manuf_data) {
            std::vector<uint8_t> raw_manuf_data;
            for (auto& elem : value_array.get_array()) {
                raw_manuf_data.push_back(elem.get_byte());
            }
            _manufacturer_data[key] = raw_manuf_data;
        }
    }
}
