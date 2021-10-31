#include "Adapter1.h"

#include "simpledbus/base/Logger.h"

const std::string Adapter1::_interface_name = "org.bluez.Adapter1";

Adapter1::Adapter1(SimpleDBus::Connection* conn, std::string path)
    : _conn(conn), _path(path), _discovering(false), Properties{conn, "org.bluez", path}, PropertyHandler(path) {}

Adapter1::~Adapter1() {}

void Adapter1::add_option(std::string option_name, SimpleDBus::Holder value) {
    if (option_name == "Discovering") {
        _discovering = value.get_boolean();
        if (_discovering && OnDiscoveryStarted) {
            LOG_F(VERBOSE_0, "%s -> OnDiscoveryStarted", _path.c_str());
            OnDiscoveryStarted();
        } else if (!_discovering && OnDiscoveryStopped) {
            LOG_F(VERBOSE_0, "%s -> OnDiscoveryStopped", _path.c_str());
            OnDiscoveryStopped();
        }
    } else if (option_name == "Address") {
        _address = value.get_string();
    }
}
void Adapter1::remove_option(std::string option_name) {}

void Adapter1::StartDiscovery() {
    if (!_discovering) {
        LOG_F(DEBUG, "%s -> StartDiscovery", _path.c_str());
        auto msg = SimpleDBus::Message::create_method_call("org.bluez", _path, _interface_name, "StartDiscovery");
        _conn->send_with_reply_and_block(msg);
    } else {
        LOG_F(WARN, "%s is already discoverying...", _path.c_str());
    }
}
void Adapter1::StopDiscovery() {
    if (_discovering) {
        LOG_F(DEBUG, "%s -> StopDiscovery", _path.c_str());
        auto msg = SimpleDBus::Message::create_method_call("org.bluez", _path, _interface_name, "StopDiscovery");
        _conn->send_with_reply_and_block(msg);
        // NOTE: It might take a few seconds until the peripheral reports that is has actually stopped discovering.
    } else {
        LOG_F(WARN, "%s was not discoverying...", _path.c_str());
    }
}

void Adapter1::Action_StartDiscovery() {
    LOG_F(DEBUG, "%s -> StartDiscovery", _path.c_str());
    auto msg = SimpleDBus::Message::create_method_call("org.bluez", _path, _interface_name, "StartDiscovery");
    _conn->send_with_reply_and_block(msg);
}

void Adapter1::Action_StopDiscovery() {
    LOG_F(DEBUG, "%s -> StopDiscovery", _path.c_str());
    auto msg = SimpleDBus::Message::create_method_call("org.bluez", _path, _interface_name, "StopDiscovery");
    _conn->send_with_reply_and_block(msg);
    // NOTE: It might take a few seconds until the peripheral reports that is has actually stopped discovering.
}

bool Adapter1::Property_Discovering() {
    auto value = Get(_interface_name, "Discovering");
    add_option("Discovering", value);
    return _discovering;
}

SimpleDBus::Holder Adapter1::GetDiscoveryFilters() {
    LOG_F(DEBUG, "%s -> GetDiscoveryFilters", _path.c_str());
    auto msg = SimpleDBus::Message::create_method_call("org.bluez", _path, _interface_name, "GetDiscoveryFilters");
    SimpleDBus::Message reply_msg = _conn->send_with_reply_and_block(msg);
    SimpleDBus::Holder discovery_filters = reply_msg.extract();
    // std::cout << discovery_filters.represent() << std::endl;
    return discovery_filters;
}

void Adapter1::SetDiscoveryFilter(SimpleDBus::Holder properties) {
    LOG_F(DEBUG, "%s -> SetDiscoveryFilters", _path.c_str());
    auto msg = SimpleDBus::Message::create_method_call("org.bluez", _path, _interface_name, "SetDiscoveryFilter");
    msg.append_argument(properties, "a{sv}");
    _conn->send_with_reply_and_block(msg);
}

std::string Adapter1::Address() { return _address; }

bool Adapter1::is_discovering() { return _discovering; }
