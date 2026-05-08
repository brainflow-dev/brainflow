#pragma once

#include <simpleble/Exceptions.h>
#include <simpleble/Types.h>

#include "AdapterBase.h"
#include "AdapterBaseTypes.h"

#include <kvn_safe_callback.hpp>

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace SimpleBLE {

class Peripheral;
class PeripheralBase;
class PeripheralMac;

/**
  This class definition acts as an abstraction layer between C++ and Objective-C.
  If Objective-C headers are included here, everything blows up.
 */
class AdapterMac : public AdapterBase {
  public:
    AdapterMac();
    virtual ~AdapterMac();

    virtual void* underlying() const override;

    virtual std::string identifier() override;
    virtual BluetoothAddress address() override;

    virtual void power_on() override;
    virtual void power_off() override;
    virtual bool is_powered() override;

    virtual void scan_start() override;
    virtual void scan_stop() override;
    virtual void scan_for(int timeout_ms) override;
    virtual bool scan_is_active() override;
    virtual std::vector<std::shared_ptr<PeripheralBase>> scan_get_results() override;

    virtual std::vector<std::shared_ptr<PeripheralBase>> get_paired_peripherals() override;

    virtual bool bluetooth_enabled() override;

    void delegate_did_discover_peripheral(void* opaque_peripheral, void* opaque_adapter,
                                          advertising_data_t advertising_data);
    void delegate_did_connect_peripheral(void* opaque_peripheral);
    void delegate_did_fail_to_connect_peripheral(void* opaque_peripheral, void* opaque_error);
    void delegate_did_disconnect_peripheral(void* opaque_peripheral, void* opaque_error);

  protected:
    /**
     * Holds a pointer to the Objective-C representation of this object.
     */
    void* opaque_internal_;

    /**
     * Holds a map of objective-c peripheral objects to their corresponding C++ objects.
     *
     * Whenever some sort of event happens on a peripheral, we need to find
     * the corresponding PeripheralBase object. The PeripheralBase objects
     * can be wrapped in disposable Peripheral objects, so there is no need
     * be careful with them.
     */
    std::map<void*, std::shared_ptr<PeripheralMac>> peripherals_;
    std::map<void*, std::shared_ptr<PeripheralMac>> seen_peripherals_;

  private:
    BluetoothAddress address() const;
};

}  // namespace SimpleBLE
