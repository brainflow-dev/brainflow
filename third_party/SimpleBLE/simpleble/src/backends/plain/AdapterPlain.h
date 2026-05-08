#pragma once

#include <simpleble/Exceptions.h>
#include <simpleble/Types.h>

#include "AdapterBase.h"

#include <kvn_safe_callback.hpp>

#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace SimpleBLE {

/**
 * Dummy adapter for testing purposes.
 */
class AdapterPlain : public AdapterBase {
  public:
    AdapterPlain();
    virtual ~AdapterPlain();

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

  private:
    std::atomic_bool is_scanning_{false};
};

}  // namespace SimpleBLE
