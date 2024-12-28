#pragma once

#include "jni/Common.hpp"

namespace SimpleBLE {
    namespace Android {

        class UUID {
    public:
            UUID();
            UUID(JNI::Object obj);

            std::string toString();

        private:
            static JNI::Class _cls;
            static jmethodID _method_toString;

            static void initialize();

            JNI::Object _obj;
        };

    } // Android
} // SimpleBLE
