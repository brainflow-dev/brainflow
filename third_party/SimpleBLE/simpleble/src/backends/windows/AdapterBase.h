#pragma once

#include <simpleble/Exceptions.h>
#include <simpleble/Peripheral.h>
#include <simpleble/Types.h>

#include "AdapterBaseTypes.h"
#include "PeripheralBase.h"

#include <kvn_safe_callback.hpp>

#include "winrt/Windows.Devices.Bluetooth.Advertisement.h"
#include "winrt/Windows.Devices.Bluetooth.h"

#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <string>
#include <vector>

using namespace winrt::Windows;
using namespace winrt::Windows::Devices::Bluetooth;
using namespace winrt::Windows::Devices::Radios;

namespace SimpleBLE {

class AdapterBase {
  public:
    AdapterBase(std::string device_id);
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
    BluetoothAdapter adapter_;
    std::string identifier_;

    struct Advertisement::BluetoothLEAdvertisementWatcher scanner_;
    winrt::event_token scanner_received_token_;
    winrt::event_token scanner_stopped_token_;

    std::atomic_bool scan_is_active_{false};
    std::condition_variable scan_stop_cv_;
    std::mutex scan_stop_mutex_;
    std::map<BluetoothAddress, std::shared_ptr<PeripheralBase>> peripherals_;
    std::map<BluetoothAddress, std::shared_ptr<PeripheralBase>> seen_peripherals_;

    void _scan_stopped_callback();
    void _scan_received_callback(advertising_data_t data);

    kvn::safe_callback<void()> callback_on_scan_start_;
    kvn::safe_callback<void()> callback_on_scan_stop_;
    kvn::safe_callback<void(Peripheral)> callback_on_scan_updated_;
    kvn::safe_callback<void(Peripheral)> callback_on_scan_found_;
};

}  // namespace SimpleBLE
