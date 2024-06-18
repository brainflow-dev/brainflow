#include <simpleble_c/adapter.h>

#include <simpleble/AdapterSafe.h>

#include <cstring>

bool simpleble_adapter_is_bluetooth_enabled(void) {
    return SimpleBLE::Safe::Adapter::bluetooth_enabled().value_or(false);
}

size_t simpleble_adapter_get_count(void) {
    return SimpleBLE::Safe::Adapter::get_adapters().value_or(std::vector<SimpleBLE::Safe::Adapter>()).size();
}

simpleble_adapter_t simpleble_adapter_get_handle(size_t index) {
    auto adapter_list = SimpleBLE::Safe::Adapter::get_adapters().value_or(std::vector<SimpleBLE::Safe::Adapter>());

    if (index >= adapter_list.size()) {
        return nullptr;
    }

    SimpleBLE::Safe::Adapter* handle = new SimpleBLE::Safe::Adapter(adapter_list[index]);
    return handle;
}

void simpleble_adapter_release_handle(simpleble_adapter_t handle) {
    if (handle == nullptr) {
        return;
    }

    SimpleBLE::Safe::Adapter* adapter = (SimpleBLE::Safe::Adapter*)handle;
    delete adapter;
}

char* simpleble_adapter_identifier(simpleble_adapter_t handle) {
    if (handle == nullptr) {
        return nullptr;
    }

    SimpleBLE::Safe::Adapter* adapter = (SimpleBLE::Safe::Adapter*)handle;
    std::string identifier = adapter->identifier().value_or("");
    char* c_identifier = (char*)malloc(identifier.size() + 1);
    strcpy(c_identifier, identifier.c_str());
    return c_identifier;
}

char* simpleble_adapter_address(simpleble_adapter_t handle) {
    if (handle == nullptr) {
        return nullptr;
    }

    SimpleBLE::Safe::Adapter* adapter = (SimpleBLE::Safe::Adapter*)handle;
    std::string address = adapter->address().value_or("");
    char* c_address = (char*)malloc(address.size() + 1);
    strcpy(c_address, address.c_str());
    return c_address;
}

simpleble_err_t simpleble_adapter_scan_start(simpleble_adapter_t handle) {
    if (handle == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::Safe::Adapter* adapter = (SimpleBLE::Safe::Adapter*)handle;
    return adapter->scan_start() ? SIMPLEBLE_SUCCESS : SIMPLEBLE_FAILURE;
}

simpleble_err_t simpleble_adapter_scan_stop(simpleble_adapter_t handle) {
    if (handle == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::Safe::Adapter* adapter = (SimpleBLE::Safe::Adapter*)handle;
    return adapter->scan_stop() ? SIMPLEBLE_SUCCESS : SIMPLEBLE_FAILURE;
}

simpleble_err_t simpleble_adapter_scan_is_active(simpleble_adapter_t handle, bool* active) {
    if (handle == nullptr || active == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::Safe::Adapter* adapter = (SimpleBLE::Safe::Adapter*)handle;

    std::optional<bool> is_active = adapter->scan_is_active();
    *active = is_active.value_or(false);
    return is_active.has_value() ? SIMPLEBLE_SUCCESS : SIMPLEBLE_FAILURE;
}

simpleble_err_t simpleble_adapter_scan_for(simpleble_adapter_t handle, int timeout_ms) {
    if (handle == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::Safe::Adapter* adapter = (SimpleBLE::Safe::Adapter*)handle;
    return adapter->scan_for(timeout_ms) ? SIMPLEBLE_SUCCESS : SIMPLEBLE_FAILURE;
}

size_t simpleble_adapter_scan_get_results_count(simpleble_adapter_t handle) {
    if (handle == nullptr) {
        return 0;
    }

    SimpleBLE::Safe::Adapter* adapter = (SimpleBLE::Safe::Adapter*)handle;
    return adapter->scan_get_results().value_or(std::vector<SimpleBLE::Safe::Peripheral>()).size();
}

simpleble_peripheral_t simpleble_adapter_scan_get_results_handle(simpleble_adapter_t handle, size_t index) {
    if (handle == nullptr) {
        return nullptr;
    }

    SimpleBLE::Safe::Adapter* adapter = (SimpleBLE::Safe::Adapter*)handle;
    auto results = adapter->scan_get_results().value_or(std::vector<SimpleBLE::Safe::Peripheral>());

    if (index >= results.size()) {
        return nullptr;
    }

    SimpleBLE::Safe::Peripheral* peripheral_handle = new SimpleBLE::Safe::Peripheral(results[index]);
    return peripheral_handle;
}

size_t simpleble_adapter_get_paired_peripherals_count(simpleble_adapter_t handle) {
    if (handle == nullptr) {
        return 0;
    }

    SimpleBLE::Safe::Adapter* adapter = (SimpleBLE::Safe::Adapter*)handle;
    return adapter->get_paired_peripherals().value_or(std::vector<SimpleBLE::Safe::Peripheral>()).size();
}

simpleble_peripheral_t simpleble_adapter_get_paired_peripherals_handle(simpleble_adapter_t handle, size_t index) {
    if (handle == nullptr) {
        return nullptr;
    }

    SimpleBLE::Safe::Adapter* adapter = (SimpleBLE::Safe::Adapter*)handle;
    auto results = adapter->get_paired_peripherals().value_or(std::vector<SimpleBLE::Safe::Peripheral>());

    if (index >= results.size()) {
        return nullptr;
    }

    SimpleBLE::Safe::Peripheral* peripheral_handle = new SimpleBLE::Safe::Peripheral(results[index]);
    return peripheral_handle;
}

simpleble_err_t simpleble_adapter_set_callback_on_scan_start(simpleble_adapter_t handle,
                                                             void (*callback)(simpleble_adapter_t, void*),
                                                             void* userdata) {
    if (handle == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::Safe::Adapter* adapter = (SimpleBLE::Safe::Adapter*)handle;

    bool success = adapter->set_callback_on_scan_start([=]() { callback(handle, userdata); });
    return success ? SIMPLEBLE_SUCCESS : SIMPLEBLE_FAILURE;
}

simpleble_err_t simpleble_adapter_set_callback_on_scan_stop(simpleble_adapter_t handle,
                                                            void (*callback)(simpleble_adapter_t, void*),
                                                            void* userdata) {
    if (handle == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::Safe::Adapter* adapter = (SimpleBLE::Safe::Adapter*)handle;

    bool success = adapter->set_callback_on_scan_stop([=]() { callback(handle, userdata); });
    return success ? SIMPLEBLE_SUCCESS : SIMPLEBLE_FAILURE;
}

simpleble_err_t simpleble_adapter_set_callback_on_scan_updated(
    simpleble_adapter_t handle, void (*callback)(simpleble_adapter_t, simpleble_peripheral_t, void*), void* userdata) {
    if (handle == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::Safe::Adapter* adapter = (SimpleBLE::Safe::Adapter*)handle;

    bool success = adapter->set_callback_on_scan_updated([=](SimpleBLE::Safe::Peripheral peripheral) {
        // Create a peripheral handle
        SimpleBLE::Safe::Peripheral* peripheral_handle = new SimpleBLE::Safe::Peripheral(peripheral);
        callback(handle, peripheral_handle, userdata);
    });
    return success ? SIMPLEBLE_SUCCESS : SIMPLEBLE_FAILURE;
}

simpleble_err_t simpleble_adapter_set_callback_on_scan_found(
    simpleble_adapter_t handle, void (*callback)(simpleble_adapter_t, simpleble_peripheral_t, void*), void* userdata) {
    if (handle == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::Safe::Adapter* adapter = (SimpleBLE::Safe::Adapter*)handle;

    bool success = adapter->set_callback_on_scan_found([=](SimpleBLE::Safe::Peripheral peripheral) {
        // Create a peripheral handle
        SimpleBLE::Safe::Peripheral* peripheral_handle = new SimpleBLE::Safe::Peripheral(peripheral);
        callback(handle, peripheral_handle, userdata);
    });
    return success ? SIMPLEBLE_SUCCESS : SIMPLEBLE_FAILURE;
}
