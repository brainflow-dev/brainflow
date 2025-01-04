#pragma once

#include <jni.h>
#include <string>
#include <vector>
#include <cstdint>

#include "GlobalRef.hpp"
#include "VM.hpp"

namespace SimpleBLE {
namespace JNI {

// Forward declarations
class Class;

class Object {
  public:
    Object() = default;

    Object(jobject obj) : _obj(obj) {
        JNIEnv* env = VM::env();
        _cls = env->GetObjectClass(obj);
    }

    Object(jobject obj, jclass cls) : _obj(obj), _cls(cls) {}

    jobject get() const { return _obj.get(); }

    explicit operator bool() const {
        return _obj.get() != nullptr;
    }

    jmethodID get_method(const char* name, const char* signature) {
        JNIEnv* env = VM::env();
        return env->GetMethodID(_cls.get(), name, signature);
    }

    template <typename... Args>
    Object call_object_method(jmethodID method, Args&&... args) {
        JNIEnv* env = VM::env();
        jobject result = env->CallObjectMethod(_obj.get(), method, std::forward<Args>(args)...);
        jclass resultClass = env->GetObjectClass(result);
        return Object(result, resultClass);
    }

    template <typename... Args>
    void call_void_method(jmethodID method, Args&&... args) {
        JNIEnv* env = VM::env();
        env->CallVoidMethod(_obj.get(), method, std::forward<Args>(args)...);
    }

    template <typename... Args>
    bool call_boolean_method(jmethodID method, Args&&... args) {
        JNIEnv* env = VM::env();
        jboolean result = env->CallBooleanMethod(_obj.get(), method, std::forward<Args>(args)...);
        return result;
    }

    template <typename... Args>
    int call_int_method(jmethodID method, Args&&... args) {
        JNIEnv* env = VM::env();
        jint result = env->CallIntMethod(_obj.get(), method, std::forward<Args>(args)...);
        return result;
    }

    template <typename... Args>
    std::string call_string_method(jmethodID method, Args&&... args) {
        JNIEnv* env = VM::env();
        jstring jstr = (jstring)env->CallObjectMethod(_obj.get(), method, std::forward<Args>(args)...);

        if (jstr == nullptr) {
            return "";
        }

        const char* c_str = env->GetStringUTFChars(jstr, nullptr);
        std::string result(c_str);
        env->ReleaseStringUTFChars(jstr, c_str);
        return result;
    }

    template<typename... Args>
    std::vector<uint8_t> call_byte_array_method(jmethodID method, Args&&... args) {
        JNIEnv* env = VM::env();
        jbyteArray jarr = (jbyteArray)env->CallObjectMethod(_obj.get(), method, std::forward<Args>(args)...);

        if (jarr == nullptr) {
            return {};
        }

        jsize len = env->GetArrayLength(jarr);
        jbyte* arr = env->GetByteArrayElements(jarr, nullptr);

        std::vector<uint8_t> result(arr, arr + len);

        env->ReleaseByteArrayElements(jarr, arr, JNI_ABORT);
        return result;
    }



    template <typename... Args>
    Object call_object_method(const char* name, const char* signature, Args&&... args) {
        JNIEnv* env = VM::env();

        jmethodID method = env->GetMethodID(_cls.get(), name, signature);
        jobject result = env->CallObjectMethod(_obj.get(), method, std::forward<Args>(args)...);

        jclass resultClass = env->GetObjectClass(result);

        return Object(result, resultClass);
    }

    template <typename... Args>
    void call_void_method(const char* name, const char* signature, Args&&... args) {
        JNIEnv* env = VM::env();

        jmethodID method = env->GetMethodID(_cls.get(), name, signature);
        env->CallVoidMethod(_obj.get(), method, std::forward<Args>(args)...);
    }

    template <typename... Args>
    bool call_boolean_method(const char* name, const char* signature, Args&&... args) {
        JNIEnv* env = VM::env();

        jmethodID method = env->GetMethodID(_cls.get(), name, signature);
        jboolean result = env->CallBooleanMethod(_obj.get(), method, std::forward<Args>(args)...);

        return result;
    }

    template <typename... Args>
    int call_int_method(const char* name, const char* signature, Args&&... args) {
        JNIEnv* env = VM::env();

        jmethodID method = env->GetMethodID(_cls.get(), name, signature);
        jint result = env->CallIntMethod(_obj.get(), method, std::forward<Args>(args)...);

        return result;
    }

    template <typename... Args>
    std::string call_string_method(const char* name, const char* signature, Args&&... args) {
        JNIEnv* env = VM::env();

        jmethodID method = env->GetMethodID(_cls.get(), name, signature);
        jstring jstr = (jstring)env->CallObjectMethod(_obj.get(), method, std::forward<Args>(args)...);

        if (jstr == nullptr) {
            return "";
        }

        const char* c_str = env->GetStringUTFChars(jstr, nullptr);
        std::string result(c_str);
        env->ReleaseStringUTFChars(jstr, c_str);
        return result;
    }

    template<typename... Args>
    std::vector<uint8_t> call_byte_array_method(const char* name, const char* signature, Args&&... args) {
        JNIEnv* env = VM::env();

        jmethodID method = env->GetMethodID(_cls.get(), name, signature);
        jbyteArray jarr = (jbyteArray)env->CallObjectMethod(_obj.get(), method, std::forward<Args>(args)...);

        if (jarr == nullptr) {
            return {};
        }

        jsize len = env->GetArrayLength(jarr);
        jbyte* arr = env->GetByteArrayElements(jarr, nullptr);

        std::vector<uint8_t> result(arr, arr + len);

        env->ReleaseByteArrayElements(jarr, arr, JNI_ABORT);
        return result;
    }

  private:
    GlobalRef<jobject> _obj;
    GlobalRef<jclass> _cls;
};

class Class {
  public:
    Class() = default;
    Class(jclass cls) : _cls(cls) {}

    jclass get() { return _cls.get(); }

    template <typename... Args>
    Object call_static_method(const char* name, const char* signature, Args&&... args) {
        JNIEnv* env = VM::env();

        jmethodID method = env->GetStaticMethodID(_cls.get(), name, signature);
        jobject obj = env->CallStaticObjectMethod(_cls.get(), method, std::forward<Args>(args)...);

        return Object(obj, _cls.get());
    }

    template <typename... Args>
    Object call_constructor(const char* signature, Args&&... args) {
        JNIEnv* env = VM::env();

        jmethodID method = env->GetMethodID(_cls.get(), "<init>", signature);
        jobject obj = env->NewObject(_cls.get(), method, std::forward<Args>(args)...);

        return Object(obj, _cls.get());
    }

    void register_natives(const JNINativeMethod* methods, int nMethods) {
        JNIEnv* env = VM::env();
        env->RegisterNatives(_cls.get(), methods, nMethods);
    }

  private:
    GlobalRef<jclass> _cls;
};

class Env {
public:
    Env() { _env = VM::env(); }
    virtual ~Env() = default;
    Env(Env& other) = delete;             // Remove the copy constructor
    void operator=(const Env&) = delete;  // Remove the copy assignment

    JNIEnv* operator->() { return _env; }

    Class find_class(const std::string& name) {
        jclass jcls = _env->FindClass(name.c_str());
        if (jcls == nullptr) {
            throw std::runtime_error("Failed to find class: " + name);
        }
        Class cls(jcls);
        _env->DeleteLocalRef(jcls);
        return cls;
    }

private:
    JNIEnv* _env = nullptr;
};

// TODO: Move these to their own namespace

struct JObjectComparator {
    // TODO: Lazy initialize jclass and jmethodID objects.

    bool operator()(const jobject& lhs, const jobject& rhs) const {
        if (lhs == nullptr && rhs == nullptr) {
            return false;  // Both are null, considered equal
        }
        if (lhs == nullptr) {
            return true;   // lhs is null, rhs is not null, lhs < rhs
        }
        if (rhs == nullptr) {
            return false;  // rhs is null, lhs is not null, lhs > rhs
        }

        JNIEnv* env = VM::env();
        if (env->IsSameObject(lhs, rhs)) {
            return false;  // Both objects are the same
        }

        // Use hashCode method to establish a consistent ordering
        // TODO: Cache all references statically for this class!
        jclass objectClass = env->FindClass("java/lang/Object");
        jmethodID hashCodeMethod = env->GetMethodID(objectClass, "hashCode", "()I");

        const jobject lhsObject = lhs;
        const jobject rhsObject = rhs;

        jint lhsHashCode = env->CallIntMethod(lhsObject, hashCodeMethod);
        jint rhsHashCode = env->CallIntMethod(rhsObject, hashCodeMethod);

        return lhsHashCode < rhsHashCode;

        // Use a unique identifier or a pointer value as the final comparison for non-equal objects
        return lhs < rhs;  // This can still be used for consistent ordering
    }
};

struct JniObjectComparator {
    // TODO: Lazy initialize jclass and jmethodID objects.

    bool operator()(const Object& lhs, const Object& rhs) const {
        // Handle null object comparisons
        if (!lhs && !rhs) {
            return false; // Both are null, considered equal
        }
        if (!lhs) {
            return true; // lhs is null, rhs is not, lhs < rhs
        }
        if (!rhs) {
            return false; // rhs is null, lhs is not, lhs > rhs
        }

        JNIEnv* env = VM::env();

        // Access the underlying jobject handles from Object instances
        jobject lhsObject = lhs.get();
        jobject rhsObject = rhs.get();

        // Check if both jobject handles refer to the same object
        if (env->IsSameObject(lhsObject, rhsObject)) {
            return false; // Both objects are the same
        }

        // Use hashCode method to establish a consistent ordering
        jclass objectClass = env->FindClass("java/lang/Object");
        jmethodID hashCodeMethod = env->GetMethodID(objectClass, "hashCode", "()I");

        jint lhsHashCode = env->CallIntMethod(lhsObject, hashCodeMethod);
        jint rhsHashCode = env->CallIntMethod(rhsObject, hashCodeMethod);

        if (lhsHashCode != rhsHashCode) {
            return lhsHashCode < rhsHashCode; // Use hash code for initial comparison
        }

        // Use a direct pointer comparison as a fallback for objects with identical hash codes
        return lhsObject < rhsObject; // This comparison is consistent within the same execution
    }
};

}  // namespace JNI
}  // namespace SimpleBLE
