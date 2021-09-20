#include <simpleble_c/adapter.h>

#include <simpleble/AdapterSafe.h>

#include <cstring>

static std::vector<SimpleBLE::Safe::Adapter> adapter_list;

size_t simpleble_adapter_get_adapter_count(void) {
    adapter_list = SimpleBLE::Safe::Adapter::get_adapters().value_or(std::vector<SimpleBLE::Safe::Adapter>());
    return adapter_list.size();
}

simpleble_adapter_t simpleble_adapter_get_adapter_handle(size_t index) {
    if (index >= adapter_list.size()) {
        return nullptr;
    }

    return &adapter_list[index];
}

const char* simpleble_adapter_identifier(simpleble_adapter_t handle) {
    if (handle == nullptr) {
        return nullptr;
    }

    SimpleBLE::Safe::Adapter* adapter = (SimpleBLE::Safe::Adapter*)handle;
    std::string identifier = adapter->identifier().value_or("");
    char* c_identifier = (char*) malloc(identifier.size() + 1);
    strcpy(c_identifier, identifier.c_str());
    return c_identifier;
}

const char* simpleble_adapter_address(simpleble_adapter_t handle) {
    if (handle == nullptr) {
        return nullptr;
    }

    SimpleBLE::Safe::Adapter* adapter = (SimpleBLE::Safe::Adapter*)handle;
    std::string address = adapter->address().value_or("");
    char* c_address = (char*) malloc(address.size() + 1);
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
    std::vector<SimpleBLE::Safe::Peripheral> results = adapter->scan_get_results().value_or(
        std::vector<SimpleBLE::Safe::Peripheral>());

    if (index >= results.size()) {
        return nullptr;
    }

    return &results[index];
}

simpleble_err_t simpleble_adapter_set_callback_on_scan_start(simpleble_adapter_t handle,
                                                             void (*callback)(simpleble_adapter_t adapter)) {
    if (handle == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::Safe::Adapter* adapter = (SimpleBLE::Safe::Adapter*)handle;

    bool success = adapter->set_callback_on_scan_start([=]() { callback(handle); });
    return success ? SIMPLEBLE_SUCCESS : SIMPLEBLE_FAILURE;
}

simpleble_err_t simpleble_adapter_set_callback_on_scan_stop(simpleble_adapter_t handle,
                                                            void (*callback)(simpleble_adapter_t adapter)) {
    if (handle == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::Safe::Adapter* adapter = (SimpleBLE::Safe::Adapter*)handle;

    bool success = adapter->set_callback_on_scan_stop([=]() { callback(handle); });
    return success ? SIMPLEBLE_SUCCESS : SIMPLEBLE_FAILURE;
}

simpleble_err_t simpleble_adapter_set_callback_on_scan_updated(simpleble_adapter_t handle,
                                                               void (*callback)(simpleble_adapter_t adapter,
                                                                                simpleble_peripheral_t peripheral)) {
    if (handle == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::Safe::Adapter* adapter = (SimpleBLE::Safe::Adapter*)handle;

    bool success = adapter->set_callback_on_scan_updated(
        [=](SimpleBLE::Safe::Peripheral peripheral) { callback(handle, &peripheral); });
    return success ? SIMPLEBLE_SUCCESS : SIMPLEBLE_FAILURE;
}

simpleble_err_t simpleble_adapter_set_callback_on_scan_found(simpleble_adapter_t handle,
                                                             void (*callback)(simpleble_adapter_t adapter,
                                                                              simpleble_peripheral_t peripheral)) {
    if (handle == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::Safe::Adapter* adapter = (SimpleBLE::Safe::Adapter*)handle;

    bool success = adapter->set_callback_on_scan_found(
        [=](SimpleBLE::Safe::Peripheral peripheral) { callback(handle, &peripheral); });
    return success ? SIMPLEBLE_SUCCESS : SIMPLEBLE_FAILURE;
}
