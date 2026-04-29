#pragma once

#include "Characteristic.h"
#include "java/lang/ArrayList.h"
#include "simplejni/Common.hpp"
#include "simplejni/Registry.hpp"

namespace Org {
namespace SimpleJavaBLE {

/**
 * @brief C++ wrapper for the org.simplejavable.Service Java class.
 *
 * This class provides basic JNI registration functionality for the Service class.
 */
class Service {
  public:
    /**
     * @brief Default constructor.
     */
    Service() = default;

    /**
     * @brief Constructor that creates a Service with the specified properties.
     *
     * @param uuid The UUID string for the service.
     * @param characteristics List of characteristics associated with this service.
     */
    Service(const SimpleJNI::String<SimpleJNI::LocalRef>& uuid,
            const Java::Util::ArrayList<SimpleJNI::LocalRef>& characteristics);

    // Implicit conversion to SimpleJNI::Object
    operator SimpleJNI::Object<SimpleJNI::ReleasableLocalRef, jobject>() const;

  private:
    // Static JNI resources, populated by Registrar during JNI_OnLoad
    static SimpleJNI::GlobalRef<jclass> _cls;
    static jmethodID _init_method;

    /**
     * @brief JNI descriptor defining the Service class.
     */
    static const SimpleJNI::JNIDescriptor descriptor;

    /**
     * @brief Automatic registration of the descriptor with Registrar.
     */
    static const SimpleJNI::AutoRegister<Service> registrar;

    SimpleJNI::Object<SimpleJNI::ReleasableLocalRef> _obj;  ///< Wrapped Java object with ReleasableLocalRef lifetime.
};

}  // namespace SimpleJavaBLE
}  // namespace Org