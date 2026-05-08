#include <simplecble/adapter.h>

#include <simpleble/Adapter.h>
#include <simpleble/Exceptions.h>

#include <cstring>
#include <vector>

bool simpleble_adapter_is_bluetooth_enabled(void) {
    try {
        return SimpleBLE::Adapter::bluetooth_enabled();
    } catch (...) {
        return false;
    }
}

size_t simpleble_adapter_get_count(void) {
    try {
        return SimpleBLE::Adapter::get_adapters().size();
    } catch (...) {
        return 0;
    }
}

simpleble_adapter_t simpleble_adapter_get_handle(size_t index) {
    try {
        auto adapter_list = SimpleBLE::Adapter::get_adapters();

        if (index >= adapter_list.size()) {
            return nullptr;
        }

        SimpleBLE::Adapter* handle = new SimpleBLE::Adapter(adapter_list[index]);
        return (simpleble_adapter_t)handle;
    } catch (...) {
        return nullptr;
    }
}

void simpleble_adapter_release_handle(simpleble_adapter_t handle) {
    if (handle == nullptr) {
        return;
    }

    SimpleBLE::Adapter* adapter = (SimpleBLE::Adapter*)handle;
    delete adapter;
}

void* simpleble_adapter_underlying(simpleble_adapter_t handle) {
    if (handle == nullptr) {
        return nullptr;
    }

    SimpleBLE::Adapter* adapter = (SimpleBLE::Adapter*)handle;
    try {
        return adapter->underlying();
    } catch (...) {
        return nullptr;
    }
}

char* simpleble_adapter_identifier(simpleble_adapter_t handle) {
    if (handle == nullptr) {
        return nullptr;
    }

    SimpleBLE::Adapter* adapter = (SimpleBLE::Adapter*)handle;
    try {
        std::string identifier = adapter->identifier();
        char* c_identifier = (char*)malloc(identifier.size() + 1);
        strcpy(c_identifier, identifier.c_str());
        return c_identifier;
    } catch (...) {
        return nullptr;
    }
}

char* simpleble_adapter_address(simpleble_adapter_t handle) {
    if (handle == nullptr) {
        return nullptr;
    }

    SimpleBLE::Adapter* adapter = (SimpleBLE::Adapter*)handle;
    try {
        std::string address = adapter->address();
        char* c_address = (char*)malloc(address.size() + 1);
        strcpy(c_address, address.c_str());
        return c_address;
    } catch (...) {
        return nullptr;
    }
}

simpleble_err_t simpleble_adapter_power_on(simpleble_adapter_t handle) {
    if (handle == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::Adapter* adapter = (SimpleBLE::Adapter*)handle;
    try {
        adapter->power_on();
        return SIMPLEBLE_SUCCESS;
    } catch (...) {
        return SIMPLEBLE_FAILURE;
    }
}

simpleble_err_t simpleble_adapter_power_off(simpleble_adapter_t handle) {
    if (handle == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::Adapter* adapter = (SimpleBLE::Adapter*)handle;
    try {
        adapter->power_off();
        return SIMPLEBLE_SUCCESS;
    } catch (...) {
        return SIMPLEBLE_FAILURE;
    }
}

simpleble_err_t simpleble_adapter_is_powered(simpleble_adapter_t handle, bool* powered) {
    if (handle == nullptr || powered == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::Adapter* adapter = (SimpleBLE::Adapter*)handle;
    try {
        *powered = adapter->is_powered();
        return SIMPLEBLE_SUCCESS;
    } catch (...) {
        return SIMPLEBLE_FAILURE;
    }
}

simpleble_err_t simpleble_adapter_set_callback_on_power_on(simpleble_adapter_t handle,
                                                           void (*callback)(simpleble_adapter_t, void*),
                                                           void* userdata) {
    if (handle == nullptr || callback == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::Adapter* adapter = (SimpleBLE::Adapter*)handle;
    try {
        adapter->set_callback_on_power_on([=]() { callback(handle, userdata); });
        return SIMPLEBLE_SUCCESS;
    } catch (...) {
        return SIMPLEBLE_FAILURE;
    }
}

simpleble_err_t simpleble_adapter_set_callback_on_power_off(simpleble_adapter_t handle,
                                                            void (*callback)(simpleble_adapter_t, void*),
                                                            void* userdata) {
    if (handle == nullptr || callback == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::Adapter* adapter = (SimpleBLE::Adapter*)handle;
    try {
        adapter->set_callback_on_power_off([=]() { callback(handle, userdata); });
        return SIMPLEBLE_SUCCESS;
    } catch (...) {
        return SIMPLEBLE_FAILURE;
    }
}

simpleble_err_t simpleble_adapter_scan_start(simpleble_adapter_t handle) {
    if (handle == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::Adapter* adapter = (SimpleBLE::Adapter*)handle;
    try {
        adapter->scan_start();
        return SIMPLEBLE_SUCCESS;
    } catch (...) {
        return SIMPLEBLE_FAILURE;
    }
}

simpleble_err_t simpleble_adapter_scan_stop(simpleble_adapter_t handle) {
    if (handle == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::Adapter* adapter = (SimpleBLE::Adapter*)handle;
    try {
        adapter->scan_stop();
        return SIMPLEBLE_SUCCESS;
    } catch (...) {
        return SIMPLEBLE_FAILURE;
    }
}

simpleble_err_t simpleble_adapter_scan_is_active(simpleble_adapter_t handle, bool* active) {
    if (handle == nullptr || active == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::Adapter* adapter = (SimpleBLE::Adapter*)handle;
    try {
        *active = adapter->scan_is_active();
        return SIMPLEBLE_SUCCESS;
    } catch (...) {
        return SIMPLEBLE_FAILURE;
    }
}

simpleble_err_t simpleble_adapter_scan_for(simpleble_adapter_t handle, int timeout_ms) {
    if (handle == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::Adapter* adapter = (SimpleBLE::Adapter*)handle;
    try {
        adapter->scan_for(timeout_ms);
        return SIMPLEBLE_SUCCESS;
    } catch (...) {
        return SIMPLEBLE_FAILURE;
    }
}

size_t simpleble_adapter_scan_get_results_count(simpleble_adapter_t handle) {
    if (handle == nullptr) {
        return 0;
    }

    SimpleBLE::Adapter* adapter = (SimpleBLE::Adapter*)handle;
    try {
        return adapter->scan_get_results().size();
    } catch (...) {
        return 0;
    }
}

simpleble_peripheral_t simpleble_adapter_scan_get_results_handle(simpleble_adapter_t handle, size_t index) {
    if (handle == nullptr) {
        return nullptr;
    }

    SimpleBLE::Adapter* adapter = (SimpleBLE::Adapter*)handle;
    try {
        auto results = adapter->scan_get_results();

        if (index >= results.size()) {
            return nullptr;
        }

        SimpleBLE::Peripheral* peripheral_handle = new SimpleBLE::Peripheral(results[index]);
        return (simpleble_peripheral_t)peripheral_handle;
    } catch (...) {
        return nullptr;
    }
}

size_t simpleble_adapter_get_paired_peripherals_count(simpleble_adapter_t handle) {
    if (handle == nullptr) {
        return 0;
    }

    SimpleBLE::Adapter* adapter = (SimpleBLE::Adapter*)handle;
    try {
        return adapter->get_paired_peripherals().size();
    } catch (...) {
        return 0;
    }
}

simpleble_peripheral_t simpleble_adapter_get_paired_peripherals_handle(simpleble_adapter_t handle, size_t index) {
    if (handle == nullptr) {
        return nullptr;
    }

    SimpleBLE::Adapter* adapter = (SimpleBLE::Adapter*)handle;
    try {
        auto results = adapter->get_paired_peripherals();

        if (index >= results.size()) {
            return nullptr;
        }

        SimpleBLE::Peripheral* peripheral_handle = new SimpleBLE::Peripheral(results[index]);
        return (simpleble_peripheral_t)peripheral_handle;
    } catch (...) {
        return nullptr;
    }
}

size_t simpleble_adapter_get_connected_peripherals_count(simpleble_adapter_t handle) {
    if (handle == nullptr) {
        return 0;
    }

    SimpleBLE::Adapter* adapter = (SimpleBLE::Adapter*)handle;
    try {
        return adapter->get_connected_peripherals().size();
    } catch (...) {
        return 0;
    }
}

simpleble_peripheral_t simpleble_adapter_get_connected_peripherals_handle(simpleble_adapter_t handle, size_t index) {
    if (handle == nullptr) {
        return nullptr;
    }

    SimpleBLE::Adapter* adapter = (SimpleBLE::Adapter*)handle;
    try {
        auto results = adapter->get_connected_peripherals();

        if (index >= results.size()) {
            return nullptr;
        }

        SimpleBLE::Peripheral* peripheral_handle = new SimpleBLE::Peripheral(results[index]);
        return (simpleble_peripheral_t)peripheral_handle;
    } catch (...) {
        return nullptr;
    }
}

simpleble_err_t simpleble_adapter_set_callback_on_scan_start(simpleble_adapter_t handle,
                                                             void (*callback)(simpleble_adapter_t, void*),
                                                             void* userdata) {
    if (handle == nullptr || callback == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::Adapter* adapter = (SimpleBLE::Adapter*)handle;
    try {
        adapter->set_callback_on_scan_start([=]() { callback(handle, userdata); });
        return SIMPLEBLE_SUCCESS;
    } catch (...) {
        return SIMPLEBLE_FAILURE;
    }
}

simpleble_err_t simpleble_adapter_set_callback_on_scan_stop(simpleble_adapter_t handle,
                                                            void (*callback)(simpleble_adapter_t, void*),
                                                            void* userdata) {
    if (handle == nullptr || callback == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::Adapter* adapter = (SimpleBLE::Adapter*)handle;
    try {
        adapter->set_callback_on_scan_stop([=]() { callback(handle, userdata); });
        return SIMPLEBLE_SUCCESS;
    } catch (...) {
        return SIMPLEBLE_FAILURE;
    }
}

simpleble_err_t simpleble_adapter_set_callback_on_scan_updated(
    simpleble_adapter_t handle, void (*callback)(simpleble_adapter_t, simpleble_peripheral_t, void*), void* userdata) {
    if (handle == nullptr || callback == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::Adapter* adapter = (SimpleBLE::Adapter*)handle;
    try {
        adapter->set_callback_on_scan_updated([=](SimpleBLE::Peripheral peripheral) {
            SimpleBLE::Peripheral* peripheral_handle = new SimpleBLE::Peripheral(peripheral);
            callback(handle, (simpleble_peripheral_t)peripheral_handle, userdata);
        });
        return SIMPLEBLE_SUCCESS;
    } catch (...) {
        return SIMPLEBLE_FAILURE;
    }
}

simpleble_err_t simpleble_adapter_set_callback_on_scan_found(
    simpleble_adapter_t handle, void (*callback)(simpleble_adapter_t, simpleble_peripheral_t, void*), void* userdata) {
    if (handle == nullptr || callback == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::Adapter* adapter = (SimpleBLE::Adapter*)handle;
    try {
        adapter->set_callback_on_scan_found([=](SimpleBLE::Peripheral peripheral) {
            SimpleBLE::Peripheral* peripheral_handle = new SimpleBLE::Peripheral(peripheral);
            callback(handle, (simpleble_peripheral_t)peripheral_handle, userdata);
        });
        return SIMPLEBLE_SUCCESS;
    } catch (...) {
        return SIMPLEBLE_FAILURE;
    }
}