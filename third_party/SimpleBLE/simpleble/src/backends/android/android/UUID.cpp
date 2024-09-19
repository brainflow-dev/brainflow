#include "UUID.h"

namespace SimpleBLE {
    namespace Android {

        JNI::Class UUID::_cls;
        jmethodID UUID::_method_toString = nullptr;

        void UUID::initialize() {
            JNI::Env env;

            if (_cls.get() == nullptr) {
                _cls = env.find_class("java/util/UUID");
            }

            if (!_method_toString) {
                _method_toString = env->GetMethodID(_cls.get(), "toString", "()Ljava/lang/String;");
            }
        }

        UUID::UUID() { initialize(); }

        UUID::UUID(JNI::Object obj) : UUID() {
            _obj = obj;
        }

        std::string UUID::toString() {
            if (!_obj) return "";

            return _obj.call_string_method(_method_toString);
        }

    } // Android
} // SimpleBLE