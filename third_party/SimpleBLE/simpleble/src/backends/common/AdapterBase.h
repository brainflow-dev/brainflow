#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <simpleble/Exceptions.h>
#include <simpleble/Types.h>

#include <kvn_safe_callback.hpp>

namespace SimpleBLE {

class Peripheral;
class PeripheralBase;

/**
 * Abstract base class for Bluetooth adapter implementations.
 *
 * Each backend must implement this class.
 *
 * Notes for implementers:
 *
 * - The methods here return shared pointers to AdapterBase, PeripheralBase,
 *   etc. The code in Adapter.cpp will automatically wrap these in Adapter,
 *   Peripheral, etc. objects.
 * - Do not use Adapter, Peripheral, etc. Those are for the user.
 * - Methods that require the adapter to be enabled will not be called by the
 *   frontend if `bluetooth_enabled()` returns false.
 */
class AdapterBase {
  public:
    virtual ~AdapterBase() = default;

    virtual void* underlying() const = 0;

    virtual std::string identifier() = 0;
    virtual BluetoothAddress address() = 0;

    virtual void power_on() = 0;
    virtual void power_off() = 0;
    virtual bool is_powered() = 0;
    virtual void set_callback_on_power_on(std::function<void()> on_power_on);
    virtual void set_callback_on_power_off(std::function<void()> on_power_off);

    virtual void scan_start() = 0;
    virtual void scan_stop() = 0;
    virtual void scan_for(int timeout_ms) = 0;
    virtual bool scan_is_active() = 0;
    virtual std::vector<std::shared_ptr<PeripheralBase>> scan_get_results() = 0;

    virtual void set_callback_on_scan_start(std::function<void()> on_scan_start);
    virtual void set_callback_on_scan_stop(std::function<void()> on_scan_stop);
    virtual void set_callback_on_scan_updated(std::function<void(Peripheral)> on_scan_updated);
    virtual void set_callback_on_scan_found(std::function<void(Peripheral)> on_scan_found);

    virtual std::vector<std::shared_ptr<PeripheralBase>> get_paired_peripherals() = 0;
    virtual std::vector<std::shared_ptr<PeripheralBase>> get_connected_peripherals() { return {}; };

    /**
     * Checks if Bluetooth is enabled.
     *
     * The enabled state may be a global setting for the system/backend, or
     * it may be specific to the adapter.
     */
    virtual bool bluetooth_enabled() = 0;

  protected:
    AdapterBase() = default;

    kvn::safe_callback<void()> _callback_on_power_on;
    kvn::safe_callback<void()> _callback_on_power_off;

    kvn::safe_callback<void()> _callback_on_scan_start;
    kvn::safe_callback<void()> _callback_on_scan_stop;
    kvn::safe_callback<void(Peripheral)> _callback_on_scan_updated;
    kvn::safe_callback<void(Peripheral)> _callback_on_scan_found;
};

}  // namespace SimpleBLE
