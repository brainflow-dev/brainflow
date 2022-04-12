#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include <simpleble/Exceptions.h>
#include <simpleble/Peripheral.h>
#include <simpleble/Types.h>

#include <simplebluez/Adapter.h>

namespace SimpleBLE {

class AdapterBase {
  public:
    AdapterBase(std::shared_ptr<SimpleBluez::Adapter> adapter);
    virtual ~AdapterBase();

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

    static std::vector<std::shared_ptr<AdapterBase>> get_adapters();

  private:
    std::shared_ptr<SimpleBluez::Adapter> adapter_;

    std::atomic_bool is_scanning_;
    std::map<BluetoothAddress, Peripheral> seen_devices_;

    std::function<void()> callback_on_scan_start_;
    std::function<void()> callback_on_scan_stop_;
    std::function<void(Peripheral)> callback_on_scan_updated_;
    std::function<void(Peripheral)> callback_on_scan_found_;
};

}  // namespace SimpleBLE
