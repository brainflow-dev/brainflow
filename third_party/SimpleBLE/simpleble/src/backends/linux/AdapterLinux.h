#pragma once

#include <simpleble/Exceptions.h>
#include <simpleble/Types.h>

#include "../common/AdapterBase.h"

#include <kvn_safe_callback.hpp>

#include <simplebluez/standard/Adapter.h>

#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace SimpleBLE {

class PeripheralLinux;

class AdapterLinux : public SimpleBLE::AdapterBase {
  public:
    AdapterLinux(std::shared_ptr<SimpleBluez::Adapter> adapter);
    virtual ~AdapterLinux();

    void* underlying() const override;

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
    std::shared_ptr<SimpleBluez::Adapter> adapter_;

    std::atomic_bool is_scanning_;

    std::map<BluetoothAddress, std::shared_ptr<PeripheralLinux>> peripherals_;
    std::map<BluetoothAddress, std::shared_ptr<PeripheralLinux>> seen_peripherals_;
};

}  // namespace SimpleBLE
