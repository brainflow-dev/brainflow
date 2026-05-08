#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <simpleble/export.h>

#include <simpleble/Exceptions.h>
#include <simpleble/Peripheral.h>
#include <simpleble/Types.h>

namespace SimpleBLE {

class AdapterBase;

/**
 * Bluetooth Adapter.
 *
 * A default-constructed Adapter object is not initialized. Calling any method
 * other than `initialized()` on an uninitialized Adapter will throw an exception
 * of type `SimpleBLE::NotInitialized`.
 *
 * NOTE: This class is intended to be used by the user only. Library developers
 * should use shared pointers to `AdapterBase` instead.
 */
class SIMPLEBLE_EXPORT Adapter {
  public:
    Adapter() = default;
    virtual ~Adapter() = default;

    bool initialized() const;

    /**
     * Retrieve the underlying OS object/handle.
     *
     * For certain compatibility with external libraries, we sometimes need to
     * expose the actual OS handle to the user. This is particularly important
     * for MacOS right now.
     */
    void* underlying() const;

    std::string identifier();
    BluetoothAddress address();

    /**
     * Control the power state of the adapter.
     *
     * NOTE: The power on/off functionality is only supported on Windows and Linux.
     *       On other platforms, this method will do nothing.
     * NOTE: Callbacks are currently a placeholder for future implementation.
     */
    void power_on();
    void power_off();
    bool is_powered();
    void set_callback_on_power_on(std::function<void()> on_power_on);
    void set_callback_on_power_off(std::function<void()> on_power_off);

    void scan_start();
    void scan_stop();
    void scan_for(int timeout_ms);
    bool scan_is_active();
    std::vector<Peripheral> scan_get_results();

    void set_callback_on_scan_start(std::function<void()> on_scan_start);
    void set_callback_on_scan_stop(std::function<void()> on_scan_stop);
    void set_callback_on_scan_updated(std::function<void(Peripheral)> on_scan_updated);
    void set_callback_on_scan_found(std::function<void(Peripheral)> on_scan_found);

    /**
     * Retrieve a list of all paired peripherals.
     *
     * NOTE:This method is currently only supported by the Linux, Windows and Android backends.
     */
    std::vector<Peripheral> get_paired_peripherals();

    /**
     * Retrieve a list of all connected peripherals.
     *
     * NOTE: This method is currently only supported by the Windows backend. (More backends coming soon.)
     */
    std::vector<Peripheral> get_connected_peripherals();

    static bool bluetooth_enabled();

    /**
     * Fetches a list of all available adapters from all available backends.
     *
     * This will cause backends to be instantiated/initialized and adapters
     * too.
     *
     * @note All configuration values must be set prior to calling this function.
     *       Please refer to the SimpleBLE::Config documentation for more details.
     */
    static std::vector<Adapter> get_adapters();

  protected:
    AdapterBase* operator->();
    const AdapterBase* operator->() const;

    std::shared_ptr<AdapterBase> internal_;
};

}  // namespace SimpleBLE
