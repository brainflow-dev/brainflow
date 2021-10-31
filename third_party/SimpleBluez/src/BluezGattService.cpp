#include "BluezGattService.h"

#include "simpledbus/base/Logger.h"

BluezGattService::BluezGattService(SimpleDBus::Connection* conn, std::string path,
                                   SimpleDBus::Holder managed_interfaces)
    : _conn(conn), _path(path), GattService1{conn, path} {
    PropertyHandler::PropertiesChanged = [&](std::string interface, SimpleDBus::Holder changed_properties,
                                        SimpleDBus::Holder invalidated_properties) {
        if (interface == "org.bluez.GattService1") {
            GattService1::set_options(changed_properties, invalidated_properties);
        } else {
        }
    };

    auto managed_interface = managed_interfaces.get_dict();
    for (auto& [iface, options] : managed_interface) {
        add_interface(iface, options);
    }
}

BluezGattService::~BluezGattService() {}

bool BluezGattService::process_received_signal(SimpleDBus::Message& message) {
    if (message.get_path() == _path) {
        if (PropertyHandler::process_received_signal(message)) return true;
    } else {
        for (auto& [gatt_char_path, gatt_characteristic] : gatt_characteristics) {
            if (gatt_characteristic->process_received_signal(message)) return true;
        }
    }
    return false;
}

void BluezGattService::add_interface(std::string interface_name, SimpleDBus::Holder options) {
    // std::cout << interface_name << std::endl;
    if (interface_name == "org.bluez.GattService1") {
        GattService1::set_options(options);
    } else {
    }
}

bool BluezGattService::add_path(std::string path, SimpleDBus::Holder options) {
    int path_elements = std::count(path.begin(), path.end(), '/');
    if (path.rfind(_path, 0) == 0) {
        if (path_elements == 6) {
            if (path.find("char") != std::string::npos) {
                // We're adding a service
                gatt_characteristics.emplace(std::make_pair(path, new BluezGattCharacteristic(_conn, path, options)));
            }
        } else {
            // Corresponds to a device component
            for (auto& [gatt_char_path, gatt_characteristic] : gatt_characteristics) {
                if (gatt_characteristic->add_path(path, options)) return true;
            }
        }
        return true;
    }
    return false;
}

bool BluezGattService::remove_path(std::string path, SimpleDBus::Holder options) {
    int path_elements = std::count(path.begin(), path.end(), '/');
    if (path.rfind(_path, 0) == 0) {
        if (path_elements == 6) {
            gatt_characteristics.erase(path);
            return true;
        } else {
            // Propagate the paths downwards until someone claims it.
            for (auto& [gatt_char_path, gatt_characteristic] : gatt_characteristics) {
                if (gatt_characteristic->remove_path(path, options)) return true;
            }
        }
    }
    return false;
}

std::vector<std::string> BluezGattService::get_characteristic_list() {
    std::vector<std::string> characteristic_list;
    for (auto& [gatt_char_path, gatt_characteristic] : gatt_characteristics) {
        characteristic_list.push_back(gatt_characteristic->get_uuid());
    }
    return characteristic_list;
}

std::shared_ptr<BluezGattCharacteristic> BluezGattService::get_characteristic(std::string char_uuid) {
    std::shared_ptr<BluezGattCharacteristic> return_value = nullptr;

    for (auto& [gatt_char_path, gatt_characteristic] : gatt_characteristics) {
        if (gatt_characteristic->get_uuid() == char_uuid) {
            return_value = gatt_characteristic;
            break;
        }
    }

    return return_value;
}
