#include "BackendAndroid.h"
#include "AdapterAndroid.h"
#include "BuildVec.h"
#include "CommonUtils.h"
#include "types/android/bluetooth/BluetoothAdapter.h"
#include "simplejni/Registry.hpp"

#include <android/log.h>
#include <fmt/core.h>

#include <string>

namespace SimpleBLE {

std::shared_ptr<BackendAndroid> BACKEND_ANDROID() { return BackendAndroid::get(); }

BackendAndroid::BackendAndroid(buildToken) {
    SimpleJNI::Registrar::get().preload(SimpleJNI::VM::env());

    _adapter = std::make_shared<AdapterAndroid>();
}

std::string BackendAndroid::name() const noexcept { return "Android"; }

SharedPtrVector<AdapterBase> BackendAndroid::get_adapters() {
    SharedPtrVector<AdapterBase> adapter_list;
    adapter_list.push_back(_adapter);
    return adapter_list;
}

bool BackendAndroid::bluetooth_enabled() {
    Android::BluetoothAdapter btAdapter = Android::BluetoothAdapter::getDefaultAdapter();

    bool isEnabled = btAdapter.isEnabled();
    int bluetoothState = btAdapter.getState();
    __android_log_write(ANDROID_LOG_INFO, "SimpleBLE", fmt::format("Bluetooth state: {}", bluetoothState).c_str());

    return isEnabled;  // bluetoothState == 12;
}

}  // namespace SimpleBLE
