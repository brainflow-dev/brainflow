#pragma once

#include "simplejni/Common.hpp"
#include "simplejni/Registry.hpp"

namespace Org {
namespace SimpleJavaBLE {

/**
 * @brief C++ wrapper for the org.simplejavable.Descriptor Java class.
 *
 * This class provides basic JNI registration functionality for the Descriptor class.
 */
class Descriptor {
  public:
    /**
     * @brief Default constructor.
     */
    Descriptor() = default;

    /**
     * @brief Constructor that takes a string UUID.
     *
     * @param uuid The UUID string for the descriptor.
     */
    explicit Descriptor(const SimpleJNI::String<SimpleJNI::LocalRef>& uuid);

    // Implicit conversion to SimpleJNI::Object
    operator SimpleJNI::Object<SimpleJNI::ReleasableLocalRef, jobject>() const;

    // Static JNI resources, populated by Registrar during JNI_OnLoad
    static SimpleJNI::GlobalRef<jclass> _cls;
    static jmethodID _init_method;

    /**
     * @brief JNI descriptor defining the Descriptor class.
     */
    static const SimpleJNI::JNIDescriptor descriptor;

    /**
     * @brief Automatic registration of the descriptor with Registrar.
     */
    static const SimpleJNI::AutoRegister<Descriptor> registrar;

    SimpleJNI::Object<SimpleJNI::ReleasableLocalRef> _obj;  ///< Wrapped Java object with ReleasableLocalRef lifetime.
};

}  // namespace SimpleJavaBLE
}  // namespace Org