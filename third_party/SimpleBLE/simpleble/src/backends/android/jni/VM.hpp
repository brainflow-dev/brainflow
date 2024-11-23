#pragma once

#include <jni.h>

#include <mutex>

namespace SimpleBLE {
namespace JNI {

// Generic class to handle the Java Virtual Machine (JVM)
class VM {
  public:
    static JavaVM* jvm() {
        static std::mutex get_mutex;       // Static mutex to ensure thread safety when accessing the logger
        std::scoped_lock lock(get_mutex);  // Unlock the mutex on function return
        static VM instance;                // Static instance of the logger to ensure proper lifecycle management

        if (instance._jvm == nullptr) {
            jsize count;
            if (JNI_GetCreatedJavaVMs(&instance._jvm, 1, &count) != JNI_OK || count == 0) {
                throw std::runtime_error("Failed to get the Java Virtual Machine");
            }
        }
        return instance._jvm;
    }

    static JNIEnv* env() {
        JNIEnv* env = nullptr;
        JavaVM* jvm = VM::jvm();
        if (jvm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
            throw std::runtime_error("Failed to get the JNIEnv");
        }
        return env;
    }

  private:
    VM() = default;
    virtual ~VM() = default;
    VM(VM& other) = delete;              // Remove the copy constructor
    void operator=(const VM&) = delete;  // Remove the copy assignment

    JavaVM* _jvm = nullptr;
};

}  // namespace JNI
}  // namespace SimpleBLE