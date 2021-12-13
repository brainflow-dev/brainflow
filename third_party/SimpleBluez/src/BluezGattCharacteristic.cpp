#include "BluezGattCharacteristic.h"

#include "simpledbus/base/Logger.h"

BluezGattCharacteristic::BluezGattCharacteristic(SimpleDBus::Connection* conn, std::string path,
                                                 SimpleDBus::Holder managed_interfaces)
    : _conn(conn), _path(path), GattCharacteristic1{conn, path} {
    PropertyHandler::PropertiesChanged = [&](std::string interface, SimpleDBus::Holder changed_properties,
                                        SimpleDBus::Holder invalidated_properties) {
        if (interface == "org.bluez.GattCharacteristic1") {
            GattCharacteristic1::set_options(changed_properties, invalidated_properties);
        } else {
        }
    };

    auto managed_interface = managed_interfaces.get_dict_string();
    for (auto& [iface, options] : managed_interface) {
        add_interface(iface, options);
    }
}

BluezGattCharacteristic::~BluezGattCharacteristic() {
    // std::cout << "Destroying BluezGattCharacteristic" << std::endl;
}

bool BluezGattCharacteristic::process_received_signal(SimpleDBus::Message& message) {
    if (message.get_path() == _path) {
        if (PropertyHandler::process_received_signal(message)) return true;
        // TODO: Add any remaining signal receivers.
    }
    return false;
}

void BluezGattCharacteristic::add_interface(std::string interface_name, SimpleDBus::Holder options) {
    // std::cout << interface_name << std::endl;
    if (interface_name == "org.bluez.GattCharacteristic1") {
        GattCharacteristic1::set_options(options);
    } else {
    }
}

bool BluezGattCharacteristic::add_path(std::string path, SimpleDBus::Holder options) {
    int path_elements = std::count(path.begin(), path.end(), '/');
    if (path.rfind(_path, 0) == 0) {
        if (path_elements == 7) {
            // TODO: Characteristics also have Descriptors, although I'm not sure we need them.
            // std::cout << "New path: " << path << std::endl << options.represent() << std::endl;
        } else {
        }

        return true;
    }
    return false;
}

bool BluezGattCharacteristic::remove_path(std::string path, SimpleDBus::Holder options) {
    LOG_F(DEBUG, "remove_path not implemented (%s needed to remove %s)", _path.c_str(), _path.c_str());
    return false;
}
