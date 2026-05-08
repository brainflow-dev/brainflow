#pragma once

#include "Descriptor.h"
#include "java/lang/ArrayList.h"
#include "simplejni/Common.hpp"
#include "simplejni/Registry.hpp"

namespace Org {
namespace SimpleJavaBLE {

/**
 * @brief C++ wrapper for the org.simplejavable.Characteristic Java class.
 *
 * This class provides basic JNI registration functionality for the Characteristic class.
 */
class Characteristic {
  public:
    /**
     * @brief Default constructor.
     */
    Characteristic() = default;

    /**
     * @brief Constructor that creates a Characteristic with the specified properties.
     *
     * @param uuid The UUID string for the characteristic.
     * @param descriptors List of descriptors associated with this characteristic.
     * @param canRead Whether the characteristic supports read operations.
     * @param canWriteRequest Whether the characteristic supports write request operations.
     * @param canWriteCommand Whether the characteristic supports write command operations.
     * @param canNotify Whether the characteristic supports notifications.
     * @param canIndicate Whether the characteristic supports indications.
     */
    Characteristic(const SimpleJNI::String<SimpleJNI::LocalRef>& uuid,
                   const Java::Util::ArrayList<SimpleJNI::LocalRef>& descriptors, bool canRead, bool canWriteRequest,
                   bool canWriteCommand, bool canNotify, bool canIndicate);

    // Implicit conversion to SimpleJNI::Object
    operator SimpleJNI::Object<SimpleJNI::ReleasableLocalRef, jobject>() const;

  private:
    // Static JNI resources, populated by Registrar during JNI_OnLoad
    static SimpleJNI::GlobalRef<jclass> _cls;
    static jmethodID _init_method;

    /**
     * @brief JNI descriptor defining the Characteristic class.
     */
    static const SimpleJNI::JNIDescriptor descriptor;

    /**
     * @brief Automatic registration of the descriptor with Registrar.
     */
    static const SimpleJNI::AutoRegister<Characteristic> registrar;

    SimpleJNI::Object<SimpleJNI::ReleasableLocalRef> _obj;  ///< Wrapped Java object with ReleasableLocalRef lifetime.
};

}  // namespace SimpleJavaBLE
}  // namespace Org