#pragma once

#include <jni.h>
#include <vector>
#include "Common.hpp"

namespace SimpleBLE {
    namespace JNI {
        namespace Types {
            // TODO: Review the inline approach some time in the future.
            inline jbyteArray toJByteArray(const std::vector<uint8_t>& data) {
                JNI::Env env;
                jbyteArray array = env->NewByteArray(data.size());
                env->SetByteArrayRegion(array, 0, data.size(), (jbyte*) data.data());
                return array;
            }

            inline std::vector<uint8_t> fromJByteArray(jbyteArray array) {
                JNI::Env env;
                jsize length = env->GetArrayLength(array);
                std::vector<uint8_t> data(length);
                env->GetByteArrayRegion(array, 0, length, (jbyte*) data.data());
                return data;
            }
        }

    }  // namespace JNI
}