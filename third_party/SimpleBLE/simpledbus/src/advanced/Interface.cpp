#include <simpledbus/advanced/Interface.h>
#include <simpledbus/advanced/Proxy.h>
#include <simpledbus/base/Exceptions.h>
#include <simpledbus/interfaces/Properties.h>

using namespace SimpleDBus;

Interface::Interface(std::shared_ptr<Connection> conn, std::shared_ptr<Proxy> proxy, const std::string& interface_name)
    : _conn(conn),
      _proxy(proxy),
      _bus_name(proxy->bus_name()),
      _path(proxy->path()),
      _interface_name(interface_name),
      _loaded(true) {}

std::shared_ptr<Proxy> Interface::proxy() const { return _proxy.lock(); }

// ----- LIFE CYCLE -----

void Interface::load(Holder options) {
    auto changed_options = options.get<std::map<std::string, Holder>>();

    // NEW PROPERTY UPDATE
    // Note: Properties that have not been defined inside _property_bases will explicitly be ignored.
    for (auto& [name, value] : changed_options) {
        if (_properties.find(name) == _properties.end()) continue;

        _properties[name]->set(value);
    }

    _loaded = true;
}

void Interface::unload() { _loaded = false; }

bool Interface::is_loaded() const { return _loaded; }

// ----- METHODS -----

Message Interface::create_method_call(const std::string& method_name) {
    return Message::create_method_call(_bus_name, _path, _interface_name, method_name);
}

// ----- PROPERTIES -----

void Interface::property_refresh(const std::string& property_name) {
    if (!_loaded || _properties.count(property_name) == 0) {
        return;
    }

    try {
        // NOTE: Due to the way Bluez handles underlying devices and the fact that
        //       they can be removed before the callback reaches back (race condition),
        //       `property_get` can sometimes fail. Because of this, the update
        //       statement is surrounded by a try-catch statement.
        auto properties_interface = std::dynamic_pointer_cast<SimpleDBus::Interfaces::Properties>(
            proxy()->interface_get("org.freedesktop.DBus.Properties"));
        Holder property_latest = properties_interface->Get(_interface_name, property_name);

        if (*_properties[property_name] != property_latest) {
            _properties[property_name]->set(property_latest);
        }
    } catch (const Exception::SendFailed& e) {
        // TODO: Log error
    }
}

void Interface::property_emit(const std::string& property_name, Holder value) {
    if (!_loaded || _properties.count(property_name) == 0) {
        return;
    }

    auto properties = std::dynamic_pointer_cast<SimpleDBus::Interfaces::Properties>(
        proxy()->interface_get("org.freedesktop.DBus.Properties"));

    std::map<std::string, SimpleDBus::Holder> changed_properties;
    changed_properties[property_name] = value;
    properties->PropertiesChanged(_interface_name, changed_properties, {});
}

bool Interface::property_exists(const std::string& property_name) { return _properties.count(property_name) > 0; }

// ----- HANDLES -----

void Interface::handle_properties_changed(Holder changed_properties, Holder invalidated_properties) {
    auto changed_options = changed_properties.get<std::map<std::string, Holder>>();
    for (auto& [name, value] : changed_options) {
        if (_properties.find(name) == _properties.end()) continue;

        _properties[name]->set(value);
    }

    auto removed_options = invalidated_properties.get<std::vector<Holder>>();
    for (auto& removed_option : removed_options) {
        if (_properties.find(removed_option.get<std::string>()) == _properties.end()) continue;

        _properties[removed_option.get<std::string>()]->invalidate();
    }
}

void Interface::handle_property_set(std::string property_name, Holder value) {
    if (_properties.find(property_name) == _properties.end()) return;

    _properties[property_name]->set(value);
}

Holder Interface::handle_property_get(std::string property_name) {
    if (_properties.find(property_name) == _properties.end()) return Holder();

    return _properties[property_name]->get();
}

Holder Interface::handle_property_get_all() {
    Holder properties = Holder::create<std::map<std::string, Holder>>();
    for (auto& [name, value] : _properties) {
        properties.dict_append(Holder::STRING, name, value->get());
    }
    return properties;
}