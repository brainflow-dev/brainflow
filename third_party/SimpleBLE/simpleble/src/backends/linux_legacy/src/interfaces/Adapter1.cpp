#include "simplebluezlegacy/interfaces/Adapter1.h"

using namespace SimpleBluezLegacy;

const SimpleDBusLegacy::AutoRegisterInterface<Adapter1> Adapter1::registry{
    "org.bluez.Adapter1",
    // clang-format off
    [](std::shared_ptr<SimpleDBusLegacy::Connection> conn, const std::string& bus_name, const std::string& path, const SimpleDBusLegacy::Holder& options) -> std::shared_ptr<SimpleDBusLegacy::Interface> {
        return std::static_pointer_cast<SimpleDBusLegacy::Interface>(std::make_shared<Adapter1>(conn, path));
    }
    // clang-format on
};

Adapter1::Adapter1(std::shared_ptr<SimpleDBusLegacy::Connection> conn, std::string path)
    : SimpleDBusLegacy::Interface(conn, "org.bluez", path, "org.bluez.Adapter1") {}

void Adapter1::StartDiscovery() {
    auto msg = create_method_call("StartDiscovery");
    _conn->send_with_reply_and_block(msg);
}

void Adapter1::StopDiscovery() {
    auto msg = create_method_call("StopDiscovery");
    _conn->send_with_reply_and_block(msg);
    // NOTE: It might take a few seconds until the peripheral reports that is has actually stopped discovering.
}

SimpleDBusLegacy::Holder Adapter1::GetDiscoveryFilters() {
    auto msg = create_method_call("GetDiscoveryFilters");
    SimpleDBusLegacy::Message reply_msg = _conn->send_with_reply_and_block(msg);
    SimpleDBusLegacy::Holder discovery_filters = reply_msg.extract();
    return discovery_filters;
}

void Adapter1::SetDiscoveryFilter(DiscoveryFilter filter) {
    SimpleDBusLegacy::Holder properties = SimpleDBusLegacy::Holder::create_dict();

    if (filter.UUIDs.size() > 0) {
        SimpleDBusLegacy::Holder uuids = SimpleDBusLegacy::Holder::create_array();
        for (size_t i = 0; i < filter.UUIDs.size(); i++) {
            uuids.array_append(SimpleDBusLegacy::Holder::create_string(filter.UUIDs.at(i)));
        }
        properties.dict_append(SimpleDBusLegacy::Holder::Type::ARRAY, "UUIDs", uuids);
    }

    if (filter.RSSI.has_value()) {
        properties.dict_append(SimpleDBusLegacy::Holder::Type::INT16, "RSSI",
                               SimpleDBusLegacy::Holder::create_int16(filter.RSSI.value()));
    }

    if (filter.Pathloss.has_value()) {
        properties.dict_append(SimpleDBusLegacy::Holder::Type::UINT16, "Pathloss",
                               SimpleDBusLegacy::Holder::create_uint16(filter.Pathloss.value()));
    }

    switch (filter.Transport) {
        case DiscoveryFilter::TransportType::AUTO: {
            properties.dict_append(SimpleDBusLegacy::Holder::Type::STRING, "Transport",
                                   SimpleDBusLegacy::Holder::create_string("auto"));
            break;
        }
        case DiscoveryFilter::TransportType::BREDR: {
            properties.dict_append(SimpleDBusLegacy::Holder::Type::STRING, "Transport",
                                   SimpleDBusLegacy::Holder::create_string("bredr"));
            break;
        }
        case DiscoveryFilter::TransportType::LE: {
            properties.dict_append(SimpleDBusLegacy::Holder::Type::STRING, "Transport",
                                   SimpleDBusLegacy::Holder::create_string("le"));
            break;
        }
    }

    if (!filter.DuplicateData) {
        properties.dict_append(SimpleDBusLegacy::Holder::Type::BOOLEAN, "DuplicateData",
                               SimpleDBusLegacy::Holder::create_boolean(false));
    }

    if (filter.Discoverable) {
        properties.dict_append(SimpleDBusLegacy::Holder::Type::BOOLEAN, "Discoverable",
                               SimpleDBusLegacy::Holder::create_boolean(false));
    }

    if (filter.Pattern.size() > 0) {
        properties.dict_append(SimpleDBusLegacy::Holder::Type::STRING, "Pattern",
                               SimpleDBusLegacy::Holder::create_string(filter.Pattern));
    }

    auto msg = create_method_call("SetDiscoveryFilter");
    msg.append_argument(properties, "a{sv}");
    _conn->send_with_reply_and_block(msg);
}

void Adapter1::RemoveDevice(std::string device_path) {
    auto msg = create_method_call("RemoveDevice");
    msg.append_argument(SimpleDBusLegacy::Holder::create_object_path(device_path), "o");
    _conn->send_with_reply_and_block(msg);
}

bool Adapter1::Discovering(bool refresh) {
    if (refresh) {
        property_refresh("Discovering");
    }

    std::scoped_lock lock(_property_update_mutex);
    return _properties["Discovering"].get_boolean();
}

bool Adapter1::Powered(bool refresh) {
    if (refresh) {
        property_refresh("Powered");
    }

    std::scoped_lock lock(_property_update_mutex);
    return _properties["Powered"].get_boolean();
}

std::string Adapter1::Address() {
    std::scoped_lock lock(_property_update_mutex);
    return _properties["Address"].get_string();
}

void Adapter1::property_changed(std::string option_name) {}
