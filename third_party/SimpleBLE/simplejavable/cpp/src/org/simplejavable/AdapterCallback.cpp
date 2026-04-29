#include "AdapterCallback.h"

namespace Org {
namespace SimpleJavaBLE {

// Define static JNI resources
SimpleJNI::GlobalRef<jclass> AdapterCallback::_cls;
jmethodID AdapterCallback::_method_on_scan_start = nullptr;
jmethodID AdapterCallback::_method_on_scan_stop = nullptr;
jmethodID AdapterCallback::_method_on_scan_updated = nullptr;
jmethodID AdapterCallback::_method_on_scan_found = nullptr;

// Define the JNI descriptor
const SimpleJNI::JNIDescriptor AdapterCallback::descriptor{
    "org/simplejavable/Adapter$Callback",  // Java interface name (inner class notation)
    &_cls,                                 // Where to store the jclass
    {                                      // Methods to preload
     {"onScanStart", "()V", &_method_on_scan_start},
     {"onScanStop", "()V", &_method_on_scan_stop},
     {"onScanUpdated", "(J)V", &_method_on_scan_updated},
     {"onScanFound", "(J)V", &_method_on_scan_found}}};

// Define the AutoRegister instance
const SimpleJNI::AutoRegister<AdapterCallback> AdapterCallback::registrar{&descriptor};

/**
 * @brief Construct a Callback from a Java object.
 *
 * Initializes the internal SimpleJNI::Object with the provided jobject and the preloaded
 * jclass. The check for _cls.get() ensures that JNI_OnLoad has run and preloaded the
 * necessary resources, throwing an exception otherwise to catch initialization issues early.
 * @param obj The Java object implementing the Callback interface.
 * @throws std::runtime_error if JNI resources are not preloaded.
 */
AdapterCallback::AdapterCallback(jobject obj) : _obj(obj, _cls.get()) {
    if (!_cls.get()) {
        throw std::runtime_error("AdapterCallback JNI resources not preloaded");
    }
}

/**
 * @brief Invoke the onScanStart method on the Java Callback object.
 *
 * Uses the preloaded _method_on_scan_start to call the Java method via SimpleJNI::Object.
 * No additional checks are needed here since the constructor enforces resource availability.
 */
void AdapterCallback::on_scan_start() {
    if (_obj.is_valid()) {
        _obj.to_local().call_void_method(_method_on_scan_start);
    }
}

/**
 * @brief Invoke the onScanStop method on the Java Callback object.
 *
 * Calls the Java onScanStop() method using the preloaded jmethodID.
 */
void AdapterCallback::on_scan_stop() {
    if (_obj.is_valid()) {
        _obj.to_local().call_void_method(_method_on_scan_stop);
    }
}

/**
 * @brief Invoke the onScanUpdated method on the Java Callback object.
 *
 * Calls the Java onScanUpdated(long) method, converting the C++ int64_t to a JNI jlong.
 * The preloaded jmethodID ensures the call is efficient and safe.
 * @param peripheral_id The ID of the peripheral that was updated.
 */
void AdapterCallback::on_scan_updated(int64_t peripheral_id) {
    if (_obj.is_valid()) {
        _obj.to_local().call_void_method(_method_on_scan_updated, static_cast<jlong>(peripheral_id));
    }
}

/**
 * @brief Invoke the onScanFound method on the Java Callback object.
 *
 * Calls the Java onScanFound(long) method with the provided peripheral ID, using the
 * preloaded jmethodID for efficiency.
 * @param peripheral_id The ID of the peripheral that was found.
 */
void AdapterCallback::on_scan_found(int64_t peripheral_id) {
    if (_obj.is_valid()) {
        _obj.to_local().call_void_method(_method_on_scan_found, static_cast<jlong>(peripheral_id));
    }
}

}  // namespace SimpleJavaBLE
}  // namespace Org