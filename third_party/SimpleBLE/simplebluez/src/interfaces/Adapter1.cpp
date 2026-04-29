#include "simplebluez/interfaces/Adapter1.h"

using namespace SimpleBluez;

const SimpleDBus::AutoRegisterInterface<Adapter1> Adapter1::registry{
    "org.bluez.Adapter1",
    // clang-format off
    [](std::shared_ptr<SimpleDBus::Connection> conn, std::shared_ptr<SimpleDBus::Proxy> proxy) -> std::shared_ptr<SimpleDBus::Interface> {
        return std::static_pointer_cast<SimpleDBus::Interface>(std::make_shared<Adapter1>(conn, proxy));
    }
    // clang-format on
};

Adapter1::Adapter1(std::shared_ptr<SimpleDBus::Connection> conn, std::shared_ptr<SimpleDBus::Proxy> proxy)
    : SimpleDBus::Interface(conn, proxy, "org.bluez.Adapter1") {}

// IMPORTANT: The destructor is defined here (instead of inline) to anchor the vtable to this object file.
// This prevents the linker from stripping this translation unit and ensures the static 'registry' variable is
// initialized at startup.
Adapter1::~Adapter1() = default;

void Adapter1::StartDiscovery() {
    auto msg = create_method_call("StartDiscovery");
    _conn->send_with_reply(msg);
}

void Adapter1::StopDiscovery() {
    auto msg = create_method_call("StopDiscovery");
    _conn->send_with_reply(msg);
    // NOTE: It might take a few seconds until the peripheral reports that is has actually stopped discovering.
}

SimpleDBus::Holder Adapter1::GetDiscoveryFilters() {
    auto msg = create_method_call("GetDiscoveryFilters");
    SimpleDBus::Message reply_msg = _conn->send_with_reply(msg);
    SimpleDBus::Holder discovery_filters = reply_msg.extract();
    return discovery_filters;
}

void Adapter1::SetDiscoveryFilter(DiscoveryFilter filter) {
    SimpleDBus::Holder properties = SimpleDBus::Holder::create<std::map<std::string, SimpleDBus::Holder>>();

    if (filter.UUIDs.size() > 0) {
        SimpleDBus::Holder uuids = SimpleDBus::Holder::create<std::vector<SimpleDBus::Holder>>();
        for (size_t i = 0; i < filter.UUIDs.size(); i++) {
            uuids.array_append(SimpleDBus::Holder::create<std::string>(filter.UUIDs.at(i)));
        }
        properties.dict_append(SimpleDBus::Holder::Type::ARRAY, "UUIDs", uuids);
    }

    if (filter.RSSI.has_value()) {
        properties.dict_append(SimpleDBus::Holder::Type::INT16, "RSSI",
                               SimpleDBus::Holder::create<int16_t>(filter.RSSI.value()));
    }

    if (filter.Pathloss.has_value()) {
        properties.dict_append(SimpleDBus::Holder::Type::UINT16, "Pathloss",
                               SimpleDBus::Holder::create<uint16_t>(filter.Pathloss.value()));
    }

    switch (filter.Transport) {
        case DiscoveryFilter::TransportType::AUTO: {
            properties.dict_append(SimpleDBus::Holder::Type::STRING, "Transport",
                                   SimpleDBus::Holder::create<std::string>("auto"));
            break;
        }
        case DiscoveryFilter::TransportType::BREDR: {
            properties.dict_append(SimpleDBus::Holder::Type::STRING, "Transport",
                                   SimpleDBus::Holder::create<std::string>("bredr"));
            break;
        }
        case DiscoveryFilter::TransportType::LE: {
            properties.dict_append(SimpleDBus::Holder::Type::STRING, "Transport",
                                   SimpleDBus::Holder::create<std::string>("le"));
            break;
        }
    }

    if (!filter.DuplicateData) {
        properties.dict_append(SimpleDBus::Holder::Type::BOOLEAN, "DuplicateData",
                               SimpleDBus::Holder::create<bool>(false));
    }

    if (filter.Discoverable) {
        properties.dict_append(SimpleDBus::Holder::Type::BOOLEAN, "Discoverable",
                               SimpleDBus::Holder::create<bool>(false));
    }

    if (filter.Pattern.size() > 0) {
        properties.dict_append(SimpleDBus::Holder::Type::STRING, "Pattern",
                               SimpleDBus::Holder::create<std::string>(filter.Pattern));
    }

    auto msg = create_method_call("SetDiscoveryFilter");
    msg.append_argument(properties, "a{sv}");
    _conn->send_with_reply(msg);
}

void Adapter1::RemoveDevice(std::string device_path) {
    auto msg = create_method_call("RemoveDevice");
    msg.append_argument(SimpleDBus::Holder::create<SimpleDBus::ObjectPath>(device_path), "o");
    _conn->send_with_reply(msg);
}
