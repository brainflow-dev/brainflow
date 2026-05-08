#include "Cache.h"

Cache& Cache::get() {
    static Cache instance;
    return instance;
}

void Cache::addAdapter(size_t adapter_id, const AdapterWrapper& adapter) {
    if (hasAdapter(adapter_id)) return;

    cached_adapters.emplace(adapter_id, adapter);
}

AdapterWrapper* Cache::getAdapter(size_t adapter_id) {
    auto it = cached_adapters.find(adapter_id);
    return (it != cached_adapters.end()) ? &it->second : nullptr;
}

bool Cache::hasAdapter(size_t adapter_id) const { return cached_adapters.find(adapter_id) != cached_adapters.end(); }

bool Cache::hasPeripheral(size_t adapter_id, size_t peripheral_id) const {
    auto adapter_it = cached_peripherals.find(adapter_id);
    return (adapter_it != cached_peripherals.end()) &&
           (adapter_it->second.find(peripheral_id) != adapter_it->second.end());
}

void Cache::addPeripheral(size_t adapter_id, size_t peripheral_id, const PeripheralWrapper& peripheral) {
    if (!hasAdapter(adapter_id)) {
        // Optional: Log or throw
        return;
    }

    if (hasPeripheral(adapter_id, peripheral_id)) {
        return;
    }

    cached_peripherals[adapter_id].emplace(peripheral_id, peripheral);
}

PeripheralWrapper* Cache::getPeripheral(size_t adapter_id, size_t peripheral_id) {
    auto adapter_it = cached_peripherals.find(adapter_id);
    if (adapter_it == cached_peripherals.end()) return nullptr;

    auto peripheral_it = adapter_it->second.find(peripheral_id);
    return (peripheral_it != adapter_it->second.end()) ? &peripheral_it->second : nullptr;
}

void Cache::removeAdapter(size_t adapter_id) {
    cached_adapters.erase(adapter_id);
    cached_peripherals.erase(adapter_id);
}

void Cache::removePeripheral(size_t adapter_id, size_t peripheral_id) {
    auto adapter_it = cached_peripherals.find(adapter_id);
    if (adapter_it != cached_peripherals.end()) {
        adapter_it->second.erase(peripheral_id);
    }
}