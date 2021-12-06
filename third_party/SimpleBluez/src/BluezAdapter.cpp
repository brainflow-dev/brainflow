#include "BluezAdapter.h"

#include "simpledbus/base/Logger.h"

#include <iostream>

BluezAdapter::BluezAdapter(SimpleDBus::Connection* conn, std::string path, SimpleDBus::Holder managed_interfaces)
    : _conn(conn), _path(path), Adapter1{conn, path}, Introspectable{conn, "org.bluez", path} {
    // std::cout << "Creating BluezAdapter" << std::endl;

    PropertyHandler::PropertiesChanged = [&](std::string interface, SimpleDBus::Holder changed_properties,
                                        SimpleDBus::Holder invalidated_properties) {
        if (interface == "org.bluez.Adapter1") {
            Adapter1::set_options(changed_properties, invalidated_properties);
        } else {
        }
    };

    auto managed_interface = managed_interfaces.get_dict_string();
    for (auto& [iface, options] : managed_interface) {
        add_interface(iface, options);
    }
}

BluezAdapter::~BluezAdapter() {
    // std::cout << "Destroying BluezAdapter" << std::endl;
}

bool BluezAdapter::process_received_signal(SimpleDBus::Message& message) {
    if (message.get_path() == _path) {
        if (PropertyHandler::process_received_signal(message)) return true;
    } else {
        for (auto& [device_path, device] : _devices) {
            if (device->process_received_signal(message)) {
                if (OnDeviceUpdated) {
                    OnDeviceUpdated(device);
                }
                return true;
            }
        }
    }
    return false;
}

void BluezAdapter::add_interface(std::string interface_name, SimpleDBus::Holder options) {
    // std::cout << interface_name << std::endl;

    if (interface_name == "org.bluez.Adapter1") {
        Adapter1::set_options(options);
    } else {
    }
}

bool BluezAdapter::add_path(std::string path, SimpleDBus::Holder options) {
    int path_elements = std::count(path.begin(), path.end(), '/');
    if (path.rfind(_path, 0) == 0) {
        if (path_elements == 4) {
            // Corresponds to a device
            _devices.emplace(std::make_pair(path, new BluezDevice(_conn, path, options)));
            if (OnDeviceFound) {
                OnDeviceFound(_devices[path]);
            }
            if (OnDeviceUpdated) {
                OnDeviceUpdated(_devices[path]);
            }
            return true;
        } else {
            // Corresponds to a device component
            for (auto& [device_path, device] : _devices) {
                if (device->add_path(path, options)) return true;
            }
        }
    }
    return false;
}

bool BluezAdapter::remove_path(std::string path, SimpleDBus::Holder options) {
    int path_elements = std::count(path.begin(), path.end(), '/');
    if (path.rfind(_path, 0) == 0) {
        if (path_elements == 4) {
            // HACK: Due to the library architecture, the only way to make sure if one
            // of the underlying objects has to be deleted is to check if the options
            // list contains `org.bluez.Device1`, `org.freedesktop.DBus.Introspectable`,
            // or `org.freedesktop.DBus.Properties`.
            // A BluezDevice might also extend from `org.bluez.Battery1`, for example.
            // Eventually this structure will have to be revisited.
            std::vector<std::string> interface_list;
            for (auto& option : options.get_array()) {
                interface_list.push_back(option.get_string());
            }
            bool must_erase = std::any_of(interface_list.begin(), interface_list.end(), [](const std::string& str) {
                return str == "org.freedesktop.DBus.Properties" || str == "org.freedesktop.DBus.Introspectable" ||
                       str == "org.bluez.Device1";
            });
            if (must_erase) {
                _devices.erase(path);
            }
            return true;
        } else {
            // Propagate the paths downwards until someone claims it.
            for (auto& [device_path, device] : _devices) {
                if (device->remove_path(path, options)) return true;
            }
        }
    }
    return false;
}

void BluezAdapter::discovery_filter_transport_set(std::string value) {
    /*
    Possible values:
        "auto"	- interleaved scan
        "bredr"	- BR/EDR inquiry
        "le"	- LE scan only
    */
    // TODO: Validate input

    SimpleDBus::Holder argument = SimpleDBus::Holder::create_dict();
    argument.dict_append(SimpleDBus::Holder::Type::STRING, "Transport", SimpleDBus::Holder::create_string(value.c_str()));
    SetDiscoveryFilter(argument);
}

std::shared_ptr<BluezDevice> BluezAdapter::get_device(std::string mac_address) {
    // TODO: Apply proper mac address formatting
    // TODO: How do I know which adapter I should check?
    std::shared_ptr<BluezDevice> return_value = nullptr;

    for (auto& [path, device] : _devices) {
        if (device->get_address() == mac_address) {
            return_value = device;
        }
    }
    return return_value;
}

std::string BluezAdapter::get_identifier() {
    std::size_t start = _path.find_last_of("/");
    return _path.substr(start + 1);
}
