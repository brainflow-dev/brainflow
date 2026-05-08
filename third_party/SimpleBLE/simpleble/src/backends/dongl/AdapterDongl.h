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

#include "AdapterBaseTypes.h"
#include "PeripheralDongl.h"
#include "serial/Protocol.h"

namespace SimpleBLE {

/**
 * Dongl adapter.
 */
class AdapterDongl : public AdapterBase {
  public:
    AdapterDongl(const std::string& device_path);
    virtual ~AdapterDongl();

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
    void _scan_received_callback(advertising_data_t data);
    void _on_simpleble_event(const simpleble_Event& event);

    std::shared_ptr<Dongl::Serial::Protocol> _serial_protocol;
    std::map<BluetoothAddress, std::shared_ptr<PeripheralDongl>> peripherals_;
    std::map<BluetoothAddress, std::shared_ptr<PeripheralDongl>> seen_peripherals_;

};

}  // namespace SimpleBLE
