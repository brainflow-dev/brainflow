#include <simpledbus/interfaces/PropertyHandler.h>

using namespace SimpleDBus::Interfaces;

PropertyHandler::PropertyHandler(std::string path) : _path(path) {}

PropertyHandler::~PropertyHandler() {}

void PropertyHandler::set_options(SimpleDBus::Holder changed_properties) {
    this->set_options(changed_properties, SimpleDBus::Holder());
}

void PropertyHandler::set_options(SimpleDBus::Holder changed_properties, SimpleDBus::Holder invalidated_properties) {
    auto changed_options = changed_properties.get_dict();
    for (auto& [name, value] : changed_options) {
        this->add_option(name, value);
    }

    auto removed_options = invalidated_properties.get_array();
    for (auto& removed_option : removed_options) {
        this->remove_option(removed_option.get_string());
    }
}

bool PropertyHandler::process_received_signal(Message& message) {
    if (message.get_path() == _path && message.is_signal("org.freedesktop.DBus.Properties", "PropertiesChanged")) {
        Holder interface = message.extract();
        message.extract_next();
        Holder changed_properties = message.extract();
        message.extract_next();
        Holder invalidated_properties = message.extract();
        if (PropertiesChanged) {
            PropertiesChanged(interface.get_string(), changed_properties, invalidated_properties);
        }
        return true;
    }
    return false;
}
