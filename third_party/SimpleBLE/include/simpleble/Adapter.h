#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <simpleble/Exceptions.h>
#include <simpleble/Peripheral.h>
#include <simpleble/Types.h>

namespace SimpleBLE {

class AdapterBase;

class Adapter {
  public:
    Adapter() = default;
    virtual ~Adapter() = default;

    bool initialized() const;
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

    static std::vector<Adapter> get_adapters();

  protected:
    std::shared_ptr<AdapterBase> internal_;
};

}  // namespace SimpleBLE
