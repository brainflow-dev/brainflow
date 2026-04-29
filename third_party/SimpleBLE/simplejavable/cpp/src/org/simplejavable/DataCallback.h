#pragma once
#include "simplejni/Common.hpp"
#include "simplejni/Registry.hpp"

namespace Org {
namespace SimpleJavaBLE {

/**
 * @brief C++ wrapper for the org.simplejavable.Peripheral$DataCallback Java interface.
 *
 * This class encapsulates a jobject representing an instance of the DataCallback interface
 * from the org.simplejavable.Peripheral class. It provides methods to invoke the interface's
 * Java methods (onDataReceived) via JNI. JNI resources are preloaded during
 * JNI_OnLoad using the AutoRegister mechanism.
 */
class DataCallback {
  public:
    DataCallback() = default;

    /**
     * @brief Construct a Callback from a Java object.
     *
     * @param obj The Java object implementing the Callback interface.
     * @throws std::runtime_error if JNI resources are not preloaded.
     */
    explicit DataCallback(jobject obj);

    /**
     * @brief Invoke the onDataReceived method on the Java DataCallback object.
     */
    void on_data_received(jbyteArray data);

  private:
    // Static JNI resources, populated by Registrar during JNI_OnLoad
    static SimpleJNI::GlobalRef<jclass> _cls;
    static jmethodID _method_on_data_received;

    static const SimpleJNI::JNIDescriptor descriptor;
    static const SimpleJNI::AutoRegister<DataCallback> registrar;

    SimpleJNI::Object<SimpleJNI::WeakRef> _obj;
};

}  // namespace SimpleJavaBLE
}  // namespace Org