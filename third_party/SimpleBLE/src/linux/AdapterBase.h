#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <set>

#include <simpleble/Exceptions.h>
#include <simpleble/Peripheral.h>
#include <simpleble/Types.h>

#include <bluezdbus/BluezAdapter.h>

namespace SimpleBLE {

class AdapterBase {
  public:   
    AdapterBase(std::shared_ptr<BluezAdapter> adapter);
    ~AdapterBase();

    std::string identifier();
    BluetoothAddress address();

    void scan_start();
    void scan_stop();
    void scan_for(int timeout_ms);
    bool scan_is_active();

    void set_callback_on_scan_start(std::function<void()> on_scan_start);
    void set_callback_on_scan_stop(std::function<void()> on_scan_stop);
    void set_callback_on_scan_updated(std::function<void(Peripheral)> on_scan_updated);
    void set_callback_on_scan_found(std::function<void(Peripheral)> on_scan_found);

    static std::vector<std::shared_ptr<AdapterBase>> get_adapters();

  private: 
    std::weak_ptr<BluezAdapter> adapter_;

    std::set<BluetoothAddress> seen_devices_;

    std::function<void()> callback_on_scan_start_;
    std::function<void()> callback_on_scan_stop_;
    std::function<void(Peripheral)> callback_on_scan_updated_;
    std::function<void(Peripheral)> callback_on_scan_found_;
};

}  // namespace SimpleBLE
