#pragma once

#include <simpleble/Exceptions.h>
#include <simpleble/Peripheral.h>
#include <simpleble/Types.h>

#include <kvn_safe_callback.hpp>

#include <simplebluez/Adapter.h>

#include <atomic>
#include <functional>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace SimpleBLE {

class AdapterBase {
  public:
    AdapterBase(std::shared_ptr<SimpleBluez::Adapter> adapter);
    virtual ~AdapterBase();

    void* underlying() const;

    std::string identifier();
    BluetoothAddress address();

    void scan_start();
    void scan_stop();
    void scan_for(int timeout_ms);
    bool scan_is_active();
    std::vector<Peripheral> scan_get_results();

    void set_callback_on_scan_start(std::function<void()> on_scan_start);
    void set_callback_on_scan_stop(std::function<void()> on_scan_stop);
    void set_callback_on_scan_updated(std::function<void(Peripheral)> on_scan_updated);
    void set_callback_on_scan_found(std::function<void(Peripheral)> on_scan_found);

    std::vector<Peripheral> get_paired_peripherals();

    static bool bluetooth_enabled();
    static std::vector<std::shared_ptr<AdapterBase>> get_adapters();

  private:
    std::shared_ptr<SimpleBluez::Adapter> adapter_;

    std::atomic_bool is_scanning_;

    std::map<BluetoothAddress, std::shared_ptr<PeripheralBase>> peripherals_;
    std::map<BluetoothAddress, std::shared_ptr<PeripheralBase>> seen_peripherals_;

    kvn::safe_callback<void()> callback_on_scan_start_;
    kvn::safe_callback<void()> callback_on_scan_stop_;
    kvn::safe_callback<void(Peripheral)> callback_on_scan_updated_;
    kvn::safe_callback<void(Peripheral)> callback_on_scan_found_;
};

}  // namespace SimpleBLE
