#include <simplebluez/Adapter.h>
#include <simplebluez/Device.h>

#include <simplebluez/interfaces/Adapter1.h>

using namespace SimpleBluez;

Adapter::Adapter(std::shared_ptr<SimpleDBus::Connection> conn, const std::string& bus_name, const std::string& path)
    : Proxy(conn, bus_name, path) {}

Adapter::~Adapter() {}

std::shared_ptr<SimpleDBus::Proxy> Adapter::path_create(const std::string& path) {
    auto child = std::make_shared<Device>(_conn, _bus_name, path);
    return std::static_pointer_cast<SimpleDBus::Proxy>(child);
}

std::shared_ptr<SimpleDBus::Interface> Adapter::interfaces_create(const std::string& interface_name) {
    if (interface_name == "org.bluez.Adapter1") {
        return std::static_pointer_cast<SimpleDBus::Interface>(std::make_shared<Adapter1>(_conn, _path));
    }

    auto interface = std::make_shared<SimpleDBus::Interface>(_conn, _bus_name, _path, interface_name);
    return std::static_pointer_cast<SimpleDBus::Interface>(interface);
}

std::shared_ptr<Adapter1> Adapter::adapter1() {
    return std::dynamic_pointer_cast<Adapter1>(interface_get("org.bluez.Adapter1"));
}

std::string Adapter::identifier() const {
    std::size_t start = _path.find_last_of("/");
    return _path.substr(start + 1);
}

std::string Adapter::address() { return adapter1()->Address(); }

bool Adapter::discovering() { return adapter1()->Discovering(); }

bool Adapter::powered() { return adapter1()->Powered(); }

void Adapter::discovery_filter(const DiscoveryFilter& filter) { adapter1()->SetDiscoveryFilter(filter); }

void Adapter::discovery_start() { adapter1()->StartDiscovery(); }

void Adapter::discovery_stop() { adapter1()->StopDiscovery(); }

std::shared_ptr<Device> Adapter::device_get(const std::string& path) {
    return std::dynamic_pointer_cast<Device>(path_get(path));
}

void Adapter::device_remove(const std::string& path) { adapter1()->RemoveDevice(path); }

void Adapter::device_remove(const std::shared_ptr<Device>& device) { adapter1()->RemoveDevice(device->path()); }

std::vector<std::shared_ptr<Device>> Adapter::device_paired_get() {
    // Traverse all child paths and return only those that are paired.
    std::vector<std::shared_ptr<Device>> paired_devices;

    for (auto& [path, child] : _children) {
        if (!child->valid()) continue;

        std::shared_ptr<Device> device = std::dynamic_pointer_cast<Device>(child);
        if (device->paired()) {
            paired_devices.push_back(device);
        }
    }

    return paired_devices;
}

void Adapter::set_on_device_updated(std::function<void(std::shared_ptr<Device> device)> callback) {
    auto on_device_updated = [this, callback](std::string child_path) {
        auto device = device_get(child_path);
        if (device) {
            callback(device);
        }
    };

    on_child_created.load(on_device_updated);
    on_child_signal_received.load(on_device_updated);
}

void Adapter::clear_on_device_updated() {
    on_child_created.unload();
    on_child_signal_received.unload();
}
