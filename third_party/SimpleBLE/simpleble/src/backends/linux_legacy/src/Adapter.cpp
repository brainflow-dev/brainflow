#include <simplebluezlegacy/Adapter.h>
#include <simplebluezlegacy/Device.h>

#include <simplebluezlegacy/interfaces/Adapter1.h>

using namespace SimpleBluezLegacy;

Adapter::Adapter(std::shared_ptr<SimpleDBusLegacy::Connection> conn, const std::string& bus_name, const std::string& path)
    : Proxy(conn, bus_name, path) {}

Adapter::~Adapter() {}

std::shared_ptr<SimpleDBusLegacy::Proxy> Adapter::path_create(const std::string& path) {
    auto child = std::make_shared<Device>(_conn, _bus_name, path);
    child->on_signal_received.load([this, child]() { _on_device_updated(child); });
    return std::static_pointer_cast<SimpleDBusLegacy::Proxy>(child);
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
    _on_device_updated.load(callback);

    on_child_created.load([this, callback](std::string child_path) {
        auto device = device_get(child_path);
        if (device) {
            _on_device_updated(device);
        }
    });
}

void Adapter::clear_on_device_updated() {
    _on_device_updated.unload();
    on_child_created.unload();
}
