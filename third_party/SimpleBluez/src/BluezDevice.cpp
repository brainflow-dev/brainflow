#include "BluezDevice.h"

#include "simpledbus/base/Logger.h"

#include <iostream>

BluezDevice::BluezDevice(SimpleDBus::Connection* conn, std::string path, SimpleDBus::Holder managed_interfaces)
    : _conn(conn), _path(path), Device1{conn, path} {
    PropertyHandler::PropertiesChanged = [&](std::string interface, SimpleDBus::Holder changed_properties,
                                        SimpleDBus::Holder invalidated_properties) {
        if (interface == "org.bluez.Device1") {
            Device1::set_options(changed_properties, invalidated_properties);
        } else {
        }
    };

    auto managed_interface = managed_interfaces.get_dict_string();
    for (auto& [iface, options] : managed_interface) {
        add_interface(iface, options);
    }
}

BluezDevice::~BluezDevice() {}

bool BluezDevice::process_received_signal(SimpleDBus::Message& message) {
    if (message.get_path() == _path) {
        if (PropertyHandler::process_received_signal(message)) return true;
    } else {
        for (auto& [gatt_service_path, gatt_service] : gatt_services) {
            if (gatt_service->process_received_signal(message)) return true;
        }
    }
    return false;
}

void BluezDevice::add_interface(std::string interface_name, SimpleDBus::Holder options) {
    if (interface_name == "org.bluez.Device1") {
        Device1::set_options(options);
    } else {
    }
}

bool BluezDevice::add_path(std::string path, SimpleDBus::Holder options) {
    int path_elements = std::count(path.begin(), path.end(), '/');
    if (path.rfind(_path, 0) == 0) {
        if (path_elements == 5) {
            if (path.find("service") != std::string::npos) {
                // We're adding a service
                gatt_services.emplace(std::make_pair(path, new BluezGattService(_conn, path, options)));
            }
        } else {
            // Corresponds to a device component
            for (auto& [gatt_service_path, gatt_service] : gatt_services) {
                if (gatt_service->add_path(path, options)) return true;
            }
        }
        return true;
    }
    return false;
}

bool BluezDevice::remove_path(std::string path, SimpleDBus::Holder options) {
    int path_elements = std::count(path.begin(), path.end(), '/');
    if (path.rfind(_path, 0) == 0) {
        if (path_elements == 5) {
            gatt_services.erase(path);
            return true;
        } else {
            // Propagate the paths downwards until someone claims it.
            for (auto& [gatt_service_path, gatt_service] : gatt_services) {
                if (gatt_service->remove_path(path, options)) return true;
            }
        }
    }
    return false;
}

std::vector<std::string> BluezDevice::get_service_list() {
    std::vector<std::string> service_list;
    for (auto& [gatt_service_path, gatt_service] : gatt_services) {
        service_list.push_back(gatt_service->get_uuid());
    }
    return service_list;
}

std::vector<std::string> BluezDevice::get_characteristic_list(std::string service_uuid) {
    std::shared_ptr<BluezGattService> service = get_service(service_uuid);
    if (service != nullptr) {
        return service->get_characteristic_list();
    }
    return std::vector<std::string>();
}

std::shared_ptr<BluezGattService> BluezDevice::get_service(std::string service_uuid) {
    std::shared_ptr<BluezGattService> return_value = nullptr;

    for (auto& [gatt_service_path, gatt_service] : gatt_services) {
        if (gatt_service->get_uuid() == service_uuid) {
            return_value = gatt_service;
            break;
        }
    }

    return return_value;
}

std::shared_ptr<BluezGattCharacteristic> BluezDevice::get_characteristic(std::string service_uuid,
                                                                         std::string char_uuid) {
    std::shared_ptr<BluezGattCharacteristic> return_value = nullptr;
    std::shared_ptr<BluezGattService> service = get_service(service_uuid);

    if (service != nullptr) {
        return_value = service->get_characteristic(char_uuid);
    }
    return return_value;
}
