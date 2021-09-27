#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <simpleble/Exceptions.h>
#include <simpleble/Peripheral.h>
#include <simpleble/Types.h>

#include "AdapterBaseTypes.h"
#include "PeripheralBase.h"

#include "winrt/Windows.Devices.Bluetooth.Advertisement.h"
#include "winrt/Windows.Devices.Bluetooth.h"

using namespace winrt::Windows;
using namespace winrt::Windows::Devices::Bluetooth;

namespace SimpleBLE {

class AdapterBase {
  public:
    AdapterBase(std::string device_id);
    ~AdapterBase();

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
    BluetoothAdapter adapter_;
    std::string identifier_;
    struct Advertisement::BluetoothLEAdvertisementWatcher scanner_;

    std::atomic_bool scan_is_active_{false};
    std::condition_variable scan_stop_cv_;
    std::mutex scan_stop_mutex_;
    std::map<BluetoothAddress, std::shared_ptr<PeripheralBase>> peripherals_;

    void _scan_stopped_callback();
    void _scan_received_callback(advertising_data_t data);

    std::function<void()> callback_on_scan_start_;
    std::function<void()> callback_on_scan_stop_;
    std::function<void(Peripheral)> callback_on_scan_updated_;
    std::function<void(Peripheral)> callback_on_scan_found_;
};

}  // namespace SimpleBLE