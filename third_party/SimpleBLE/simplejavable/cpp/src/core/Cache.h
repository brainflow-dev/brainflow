#pragma once
#include <map>

#include "AdapterWrapper.h"
#include "PeripheralWrapper.h"

class Cache {
  private:
    Cache() = default;

    // Maps now store wrappers
    std::map<size_t, AdapterWrapper> cached_adapters;
    std::map<size_t, std::map<size_t, PeripheralWrapper>> cached_peripherals;

  public:
    static Cache& get();

    Cache(const Cache&) = delete;
    Cache& operator=(const Cache&) = delete;

    // Adapter management
    void addAdapter(size_t adapter_id, const AdapterWrapper& adapter);
    AdapterWrapper* getAdapter(size_t adapter_id);
    bool hasAdapter(size_t adapter_id) const;

    // Peripheral management
    void addPeripheral(size_t adapter_id, size_t peripheral_id, const PeripheralWrapper& peripheral);
    PeripheralWrapper* getPeripheral(size_t adapter_id, size_t peripheral_id);
    bool hasPeripheral(size_t adapter_id, size_t peripheral_id) const;

    // Removal methods
    void removeAdapter(size_t adapter_id);
    void removePeripheral(size_t adapter_id, size_t peripheral_id);
};