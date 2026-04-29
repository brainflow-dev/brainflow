#include "BluetoothAdapter.h"
#include "types/java/util/Set.h"

namespace SimpleBLE {
namespace Android {

// Static member initialization
SimpleJNI::GlobalRef<jclass> BluetoothAdapter::_cls;
jmethodID BluetoothAdapter::_method_getName = nullptr;
jmethodID BluetoothAdapter::_method_getAddress = nullptr;
jmethodID BluetoothAdapter::_method_isEnabled = nullptr;
jmethodID BluetoothAdapter::_method_getState = nullptr;
jmethodID BluetoothAdapter::_method_getBluetoothLeScanner = nullptr;
jmethodID BluetoothAdapter::_method_getBondedDevices = nullptr;
jmethodID BluetoothAdapter::_method_getDefaultAdapter = nullptr;

// JNI descriptors for auto-registration
const SimpleJNI::JNIDescriptor BluetoothAdapter::instance_descriptor{
    "android/bluetooth/BluetoothAdapter",  // Java class name
    &_cls,                                 // Where to store the jclass
    {                                      // Instance methods to preload
        {"getName", "()Ljava/lang/String;", &_method_getName},
        {"getAddress", "()Ljava/lang/String;", &_method_getAddress},
        {"isEnabled", "()Z", &_method_isEnabled},
        {"getState", "()I", &_method_getState},
        {"getBluetoothLeScanner", "()Landroid/bluetooth/le/BluetoothLeScanner;", &_method_getBluetoothLeScanner},
        {"getBondedDevices", "()Ljava/util/Set;", &_method_getBondedDevices}
    }
};

const SimpleJNI::StaticJNIDescriptor BluetoothAdapter::static_descriptor{
    "android/bluetooth/BluetoothAdapter",  // Java class name
    &_cls,                                 // Where to store the jclass (shared with instance)
    {                                      // Static methods to preload
        {"getDefaultAdapter", "()Landroid/bluetooth/BluetoothAdapter;", &_method_getDefaultAdapter}
    }
};

// Auto-register both descriptors
const SimpleJNI::AutoRegister<BluetoothAdapter> BluetoothAdapter::registrar{&instance_descriptor, &static_descriptor};

BluetoothAdapter::BluetoothAdapter(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> obj) : _obj(obj) {
    if (!obj) {
        throw std::runtime_error("BluetoothAdapter object is null");
    }
}

BluetoothAdapter BluetoothAdapter::getDefaultAdapter() {
    if (!_cls.get()) {
        throw std::runtime_error("BluetoothAdapter JNI resources not preloaded. Ensure SimpleJNI::Registrar::preload() is called.");
    }
    SimpleJNI::Env env;
    jobject local_obj = env->CallStaticObjectMethod(_cls.get(), _method_getDefaultAdapter);
    if (local_obj == nullptr) {
        throw std::runtime_error("Failed to get default BluetoothAdapter");
    }
    SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> obj(local_obj);
    env->DeleteLocalRef(local_obj);
    return BluetoothAdapter(obj);
}

std::string BluetoothAdapter::getName() {
    if (!_obj) throw std::runtime_error("BluetoothAdapter is not initialized");
    return _obj.call_string_method(_method_getName);
}

std::string BluetoothAdapter::getAddress() {
    if (!_obj) throw std::runtime_error("BluetoothAdapter is not initialized");
    return _obj.call_string_method(_method_getAddress);
}

bool BluetoothAdapter::isEnabled() {
    if (!_obj) throw std::runtime_error("BluetoothAdapter is not initialized");
    return _obj.call_boolean_method(_method_isEnabled);
}

int BluetoothAdapter::getState() {
    if (!_obj) throw std::runtime_error("BluetoothAdapter is not initialized");
    return _obj.call_int_method(_method_getState);
}

BluetoothScanner BluetoothAdapter::getBluetoothLeScanner() {
    if (!_obj) throw std::runtime_error("BluetoothAdapter is not initialized");
    auto scanner_obj = _obj.call_object_method(_method_getBluetoothLeScanner);
    return BluetoothScanner(scanner_obj);
}

std::vector<BluetoothDevice> BluetoothAdapter::getBondedDevices() {
    if (!_obj) throw std::runtime_error("BluetoothAdapter is not initialized");

    auto devices_obj = _obj.call_object_method(_method_getBondedDevices);
    if (!devices_obj) throw std::runtime_error("Failed to get bonded devices");

    std::vector<BluetoothDevice> result;
    Set set(devices_obj.to_global());
    Iterator iterator = set.iterator();
    while (iterator.hasNext()) {
        auto device_obj = iterator.next();
        if (!device_obj) continue;
        result.push_back(BluetoothDevice(device_obj));
    }

    return result;
}

}  // namespace Android
}  // namespace SimpleBLE
