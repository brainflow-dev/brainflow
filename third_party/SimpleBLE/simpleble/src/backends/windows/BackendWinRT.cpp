#include "BackendWinRT.h"

#include "AdapterWindows.h"
#include "CommonUtils.h"

#include "winrt/Windows.Devices.Bluetooth.h"
#include "winrt/Windows.Devices.Enumeration.h"
#include "winrt/Windows.Devices.Radios.h"
#include "winrt/Windows.Foundation.Collections.h"

#include "simpleble/Config.h"
#include "Utils.h"
#include "MtaManager.h"

namespace SimpleBLE {

using namespace SimpleBLE::WinRT;

std::shared_ptr<BackendBase> BACKEND_WINDOWS() { return BackendWinRT::get(); }

BackendWinRT::BackendWinRT(buildToken) {
    if (Config::WinRT::experimental_reinitialize_winrt_apartment_on_main_thread) {
        initialize_winrt();
    }
 }

bool BackendWinRT::bluetooth_enabled() {
    return MtaManager::get().execute_sync<bool>([this]() {
        bool enabled = false;
        auto radio_collection = async_get(Radio::GetRadiosAsync());
        for (uint32_t i = 0; i < radio_collection.Size(); i++) {
            auto radio = radio_collection.GetAt(i);

            // Skip non-bluetooth radios
            if (radio.Kind() != RadioKind::Bluetooth) {
                continue;
            }

            // Assume that bluetooth is enabled if any of the radios are enabled
            if (radio.State() == RadioState::On) {
                enabled = true;
                break;
            }
        }

        return enabled;
    });
}

SharedPtrVector<AdapterBase> BackendWinRT::get_adapters() {
    return MtaManager::get().execute_sync<SharedPtrVector<AdapterBase>>([this]() {
        auto device_selector = BluetoothAdapter::GetDeviceSelector();
        auto device_information_collection = async_get(
            Devices::Enumeration::DeviceInformation::FindAllAsync(device_selector));

        SharedPtrVector<AdapterBase> adapter_list;
        for (uint32_t i = 0; i < device_information_collection.Size(); i++) {
            auto dev_info = device_information_collection.GetAt(i);
            adapter_list.push_back(std::make_shared<AdapterWindows>(winrt::to_string(dev_info.Id())));
        }
        return adapter_list;
    });
}

std::string BackendWinRT::name() const noexcept { return "WinRT"; }

};  // namespace SimpleBLE
