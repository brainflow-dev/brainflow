#include "simplebluez/interfaces/Adapter1.h"

using namespace SimpleBluez;

Adapter1::Adapter1(std::shared_ptr<SimpleDBus::Connection> conn, std::string path)
    : SimpleDBus::Interface(conn, "org.bluez", path, "org.bluez.Adapter1") {}

void Adapter1::StartDiscovery() {
    auto msg = create_method_call("StartDiscovery");
    _conn->send_with_reply_and_block(msg);
}

void Adapter1::StopDiscovery() {
    auto msg = create_method_call("StopDiscovery");
    _conn->send_with_reply_and_block(msg);
    // NOTE: It might take a few seconds until the peripheral reports that is has actually stopped discovering.
}

SimpleDBus::Holder Adapter1::GetDiscoveryFilters() {
    auto msg = create_method_call("GetDiscoveryFilters");
    SimpleDBus::Message reply_msg = _conn->send_with_reply_and_block(msg);
    SimpleDBus::Holder discovery_filters = reply_msg.extract();
    return discovery_filters;
}

void Adapter1::SetDiscoveryFilter(DiscoveryFilter filter) {
    SimpleDBus::Holder properties = SimpleDBus::Holder::create_dict();

    switch (filter) {
        case DiscoveryFilter::AUTO: {
            properties.dict_append(SimpleDBus::Holder::Type::STRING, "Transport",
                                   SimpleDBus::Holder::create_string("auto"));
            break;
        }
        case DiscoveryFilter::BREDR: {
            properties.dict_append(SimpleDBus::Holder::Type::STRING, "Transport",
                                   SimpleDBus::Holder::create_string("bredr"));
            break;
        }
        case DiscoveryFilter::LE: {
            properties.dict_append(SimpleDBus::Holder::Type::STRING, "Transport",
                                   SimpleDBus::Holder::create_string("le"));
            break;
        }
    }

    auto msg = create_method_call("SetDiscoveryFilter");
    msg.append_argument(properties, "a{sv}");
    _conn->send_with_reply_and_block(msg);
}

void Adapter1::RemoveDevice(std::string device_path) {
    auto msg = create_method_call("RemoveDevice");
    msg.append_argument(SimpleDBus::Holder::create_object_path(device_path), "o");
    _conn->send_with_reply_and_block(msg);
}

bool Adapter1::Discovering(bool refresh) {
    if (refresh) {
        property_refresh("Discovering");
    }

    std::scoped_lock lock(_property_update_mutex);
    return _properties["Discovering"].get_boolean();
}

std::string Adapter1::Address() {
    std::scoped_lock lock(_property_update_mutex);
    return _properties["Address"].get_string();
}

void Adapter1::property_changed(std::string option_name) {}
