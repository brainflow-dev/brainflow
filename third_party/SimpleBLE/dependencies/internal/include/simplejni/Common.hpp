#pragma once

#include <jni.h>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>

#include "References.hpp"
#include "Registry.hpp"
#include "VM.hpp"
#include "kvn/kvn_bytearray.h"

namespace SimpleJNI {

// TODO: Implement a base class that handles common functionality of complicated Objects (see the ones in
// org/simplejavable)

class Exception : public std::runtime_error {
  public:
    Exception(jthrowable obj) : std::runtime_error("Java Exception"), _ref(), _cls() {
        _ref = LocalRef<jthrowable>(obj);

        JNIEnv* env = SimpleJNI::VM::env();
        _cls = LocalRef<jclass>(env->GetObjectClass(_ref.get()));

        jmethodID method_get_name = env->GetMethodID(_cls.get(), "getName", "()Ljava/lang/String;");
        jmethodID method_get_message = env->GetMethodID(_cls.get(), "getMessage", "()Ljava/lang/String;");

        std::string exception_name = call_string_method(env, _ref.get(), method_get_name);
        std::string exception_message = call_string_method(env, _ref.get(), method_get_message);

        _what = "Java Exception: " + exception_name + ": " + exception_message;
    }

    const char* what() const noexcept override { return _what.c_str(); }

  private:
    std::string _what;
    LocalRef<jthrowable> _ref;
    LocalRef<jclass> _cls;

    static std::string call_string_method(JNIEnv* env, jthrowable obj, jmethodID method) {
        auto jstr = static_cast<jstring>(env->CallObjectMethod(obj, method));
        if (!jstr) return "";
        const char* c_str = env->GetStringUTFChars(jstr, nullptr);
        std::string result(c_str);
        env->ReleaseStringUTFChars(jstr, c_str);
        return result;
    }
};

template <template <typename> class RefType, typename JniType = jobject>
class Object {
  public:
    Object() = default;

    explicit Object(JniType obj) : _ref(obj) {
        if (obj) {
            JNIEnv* env = VM::env();
            _cls = RefType<jclass>(env->GetObjectClass(_ref.get()));
        }
    }

    // Constructor with pre-fetched jclass
    Object(JniType obj, jclass cls) : _ref(obj), _cls(cls) {}

    // Move semantics
    Object(Object&& other) noexcept : _ref(std::move(other._ref)), _cls(std::move(other._cls)) {}
    Object& operator=(Object&& other) noexcept {
        if (this != &other) {
            _ref = std::move(other._ref);
            _cls = std::move(other._cls);
        }
        return *this;
    }

    // Copying depends on RefType's behavior (enabled by default)
    Object(const Object&) = default;
    Object& operator=(const Object&) = default;

    // Template constructor for converting between different RefType templates
    template <template <typename> class OtherRefType>
    Object(const Object<OtherRefType, JniType>& other) {
        if (other.get()) {
            JNIEnv* env = VM::env();
            _ref = RefType<JniType>(other.get());
            _cls = RefType<jclass>(env->GetObjectClass(_ref.get()));
        }
    }

    // Template assignment operator for converting between different RefType templates
    template <template <typename> class OtherRefType>
    Object& operator=(const Object<OtherRefType, JniType>& other) {
        // Don't use pointer comparison for different template types
        // Instead, check if the underlying JNI objects are the same
        JNIEnv* env = VM::env();

        // Only proceed with assignment if the objects are different
        // or if other is null (in which case we reset this object)
        if (!other.get() || !_ref.get() || !env->IsSameObject(_ref.get(), other.get())) {
            if (other.get()) {
                _ref = RefType<JniType>(other.get());
                _cls = RefType<jclass>(env->GetObjectClass(_ref.get()));
            } else {
                _ref = RefType<JniType>();
                _cls = RefType<jclass>();
            }
        }
        return *this;
    }

    // Conversion methods
    Object<LocalRef, JniType> to_local() const {
        if (!*this) return Object<LocalRef, JniType>();
        JNIEnv* env = VM::env();
        return Object<LocalRef, JniType>(_ref.get(), _cls.get());
    }

    Object<GlobalRef, JniType> to_global() const {
        if (!*this) return Object<GlobalRef, JniType>();
        JNIEnv* env = VM::env();
        return Object<GlobalRef, JniType>(_ref.get(), _cls.get());
    }

    Object<WeakRef, jweak> to_weak() const {
        if (!*this) return Object<WeakRef, jweak>();
        JNIEnv* env = VM::env();
        return Object<WeakRef, jweak>(WeakRef<jweak>(_ref.get()));
    }

    // Access raw jobject
    JniType get() const { return _ref.get(); }

    // Release ownership of the underlying reference
    JniType release() noexcept { return _ref.release(); }

    explicit operator bool() const { return _ref.get() != nullptr; }

    bool is_valid() const { return _ref.is_valid(); }

    jmethodID get_method(const char* name, const char* signature) const {
        JNIEnv* env = VM::env();
        jmethodID method = env->GetMethodID(_cls.get(), name, signature);
        check_exception(env);
        return method;
    }

    template <typename... Args>
    Object<LocalRef, JniType> call_object_method(jmethodID method, Args&&... args) const {
        JNIEnv* env = VM::env();
        JniType result = env->CallObjectMethod(_ref.get(), method, std::forward<Args>(args)...);
        check_exception(env);
        return Object<LocalRef, JniType>(result);
    }

    template <typename... Args>
    void call_void_method(jmethodID method, Args&&... args) const {
        JNIEnv* env = VM::env();
        env->CallVoidMethod(_ref.get(), method, std::forward<Args>(args)...);
        check_exception(env);
    }

    template <typename... Args>
    bool call_boolean_method(jmethodID method, Args&&... args) const {
        JNIEnv* env = VM::env();
        bool result = env->CallBooleanMethod(_ref.get(), method, std::forward<Args>(args)...);
        check_exception(env);
        return result;
    }

    template <typename... Args>
    int call_int_method(jmethodID method, Args&&... args) const {
        JNIEnv* env = VM::env();
        int result = env->CallIntMethod(_ref.get(), method, std::forward<Args>(args)...);
        check_exception(env);
        return result;
    }

    template <typename... Args>
    std::string call_string_method(jmethodID method, Args&&... args) const {
        JNIEnv* env = VM::env();
        auto jstr = static_cast<jstring>(env->CallObjectMethod(_ref.get(), method, std::forward<Args>(args)...));
        if (!jstr) return "";
        const char* c_str = env->GetStringUTFChars(jstr, nullptr);
        std::string result(c_str);
        env->ReleaseStringUTFChars(jstr, c_str);
        return result;
    }

    template <typename... Args>
    kvn::bytearray call_byte_array_method(jmethodID method, Args&&... args) const {
        JNIEnv* env = VM::env();
        auto jarr = static_cast<jbyteArray>(env->CallObjectMethod(_ref.get(), method, std::forward<Args>(args)...));
        check_exception(env);
        if (!jarr) return {};
        jsize len = env->GetArrayLength(jarr);
        jbyte* arr = env->GetByteArrayElements(jarr, nullptr);
        kvn::bytearray result(arr, arr + len);
        env->ReleaseByteArrayElements(jarr, arr, JNI_ABORT);
        return result;
    }

    template <typename... Args>
    static Object<LocalRef, JniType> call_new_object(jclass cls, jmethodID method, Args&&... args) {
        JNIEnv* env = VM::env();
        JniType result = env->NewObject(cls, method, std::forward<Args>(args)...);
        check_exception(env);
        return Object<LocalRef, JniType>(result);
    }

    static void check_exception(JNIEnv* env) {
        if (env->ExceptionCheck()) {
            Object<LocalRef, jthrowable> exception(env->ExceptionOccurred());
            env->ExceptionClear();
            throw Exception(exception.get());
        }
    }

  protected:
    RefType<JniType> _ref;  // Holds LocalRef<JniType> or GlobalRef<JniType>
    RefType<jclass> _cls;   // Holds LocalRef<jclass> or GlobalRef<jclass>
};

template <template <typename> class RefType>
class ByteArray {
  public:
    ByteArray() = default;

    // NOTE: The user is responsible for ensuring that the jobject is a jbyteArray
    explicit ByteArray(jobject obj) : _ref(static_cast<jbyteArray>(obj)) {
        JNIEnv* env = VM::env();
        jclass cls = env->FindClass("java/lang/Object");
        this->_cls = RefType<jclass>(cls);
        env->DeleteLocalRef(cls);
    }

    explicit ByteArray(jbyteArray obj) : _ref(obj) {
        JNIEnv* env = VM::env();
        jclass cls = env->FindClass("java/lang/Object");
        this->_cls = RefType<jclass>(cls);
        env->DeleteLocalRef(cls);
    }

    ByteArray(const kvn::bytearray& data) : _ref() {
        JNIEnv* env = VM::env();
        jbyteArray jarr = env->NewByteArray(data.size());
        env->SetByteArrayRegion(jarr, 0, data.size(), reinterpret_cast<const jbyte*>(data.data()));

        this->_ref = RefType<jbyteArray>(jarr);
        jclass cls = env->FindClass("java/lang/Object");
        this->_cls = RefType<jclass>(cls);
        env->DeleteLocalRef(cls);
    }

    template <template <typename> class OtherRefType>
    ByteArray(const Object<OtherRefType, jbyteArray>& obj) : _ref(obj.get()) {}

    // Add implicit conversion to Object<RefType, jobject>
    operator Object<RefType, jobject>() const {
        return Object<RefType, jobject>(static_cast<jobject>(this->get()), this->_cls.get());
    }

    // Access raw jobject
    jbyteArray get() const { return _ref.get(); }

    // Release ownership of the underlying reference
    jbyteArray release() noexcept { return _ref.release(); }

    // Conversion methods
    ByteArray<LocalRef> to_local() const {
        if (!*this) return ByteArray<LocalRef>();
        return ByteArray<LocalRef>(this->get());
    }

    ByteArray<GlobalRef> to_global() const {
        if (!*this) return ByteArray<GlobalRef>();
        return ByteArray<GlobalRef>(this->get());
    }

    explicit operator bool() const { return _ref.get() != nullptr; }

    // Get the raw byte array data
    kvn::bytearray bytes() const {
        JNIEnv* env = VM::env();
        jbyteArray jarr = this->get();

        if (jarr == nullptr) {
            return {};
        }

        jsize len = env->GetArrayLength(jarr);
        kvn::bytearray result(len);

        env->GetByteArrayRegion(jarr, 0, len, reinterpret_cast<jbyte*>(result.data()));

        return result;
    }

    // Get the length of the byte array
    size_t length() const {
        JNIEnv* env = VM::env();
        return env->GetArrayLength(this->get());
    }

  protected:
    RefType<jbyteArray> _ref;
    RefType<jclass> _cls;
};

template <template <typename> class RefType>
class LongArray {
  public:
    LongArray() = default;

    explicit LongArray(jlongArray obj) : _ref(obj) {
        JNIEnv* env = VM::env();
        jclass cls = env->FindClass("java/lang/Object");
        this->_cls = RefType<jclass>(cls);
        env->DeleteLocalRef(cls);
    }

    LongArray(const std::vector<int64_t>& data) : _ref() {
        JNIEnv* env = VM::env();
        jlongArray jarr = env->NewLongArray(data.size());
        env->SetLongArrayRegion(jarr, 0, data.size(), reinterpret_cast<const jlong*>(data.data()));

        this->_ref = RefType<jlongArray>(jarr);
        jclass cls = env->FindClass("java/lang/Object");
        this->_cls = RefType<jclass>(cls);
        env->DeleteLocalRef(cls);
    }

    template <template <typename> class OtherRefType>
    LongArray(const Object<OtherRefType, jlongArray>& obj) : _ref(obj.get()) {}

    // Add implicit conversion to Object<RefType, jobject>
    operator Object<RefType, jobject>() const {
        return Object<RefType, jobject>(static_cast<jobject>(this->get()), this->_cls.get());
    }

    // Access raw jobject
    jlongArray get() const { return _ref.get(); }

    // Release ownership of the underlying reference
    jlongArray release() noexcept { return _ref.release(); }

    // Conversion methods
    LongArray<LocalRef> to_local() const {
        if (!*this) return LongArray<LocalRef>();
        return LongArray<LocalRef>(this->get());
    }

    LongArray<GlobalRef> to_global() const {
        if (!*this) return LongArray<GlobalRef>();
        return LongArray<GlobalRef>(this->get());
    }

    explicit operator bool() const { return _ref.get() != nullptr; }

    // Get the raw long array data
    std::vector<int64_t> longs() const {
        JNIEnv* env = VM::env();
        jlongArray jarr = this->get();

        if (jarr == nullptr) {
            return {};
        }

        jsize len = env->GetArrayLength(jarr);
        std::vector<int64_t> result(len);

        env->GetLongArrayRegion(jarr, 0, len, reinterpret_cast<jlong*>(result.data()));

        return result;
    }

    // Get the length of the long array
    size_t length() const {
        JNIEnv* env = VM::env();
        jlongArray jarr = this->get();

        if (jarr == nullptr) {
            return 0;  // Return 0 for null arrays
        }

        return env->GetArrayLength(jarr);
    }

  protected:
    RefType<jlongArray> _ref;
    RefType<jclass> _cls;
};

template <template <typename> class RefType>
class String {
  public:
    String() = default;

    explicit String(jstring obj) : _ref(obj) {
        JNIEnv* env = VM::env();
        jclass cls = env->FindClass("java/lang/Object");
        this->_cls = RefType<jclass>(cls);
        env->DeleteLocalRef(cls);
    }

    String(const std::string& data) : _ref() {
        JNIEnv* env = VM::env();
        jstring jstr = env->NewStringUTF(data.c_str());

        this->_ref = RefType<jstring>(jstr);
        jclass cls = env->FindClass("java/lang/Object");
        this->_cls = RefType<jclass>(cls);
        env->DeleteLocalRef(cls);
    }

    template <template <typename> class OtherRefType>
    String(const Object<OtherRefType, jstring>& obj) : _ref(obj.get()) {}

    // Add implicit conversion to Object<RefType, jobject>
    operator Object<RefType, jobject>() const {
        return Object<RefType, jobject>(static_cast<jobject>(this->get()), this->_cls.get());
    }

    explicit operator bool() const { return _ref.get() != nullptr; }

    // Access raw jobject
    jstring get() const { return _ref.get(); }

    // Release ownership of the underlying reference
    jstring release() noexcept { return _ref.release(); }

    // Conversion methods
    String<LocalRef> to_local() const {
        if (!*this) return String<LocalRef>();
        return String<LocalRef>(this->get());
    }

    String<GlobalRef> to_global() const {
        if (!*this) return String<GlobalRef>();
        return String<GlobalRef>(this->get());
    }

    // Get the raw string data
    std::string str() const {
        JNIEnv* env = VM::env();
        jstring jstr = this->get();

        if (jstr == nullptr) {
            return {};
        }

        const char* c_str = env->GetStringUTFChars(jstr, nullptr);
        std::string result(c_str);
        env->ReleaseStringUTFChars(jstr, c_str);

        return result;
    }

    // Get the length of the string
    size_t length() const {
        JNIEnv* env = VM::env();
        return env->GetStringUTFLength(this->get());
    }

  protected:
    RefType<jstring> _ref;
    RefType<jclass> _cls;
};

class Env {
  public:
    Env() { _env = VM::env(); }
    virtual ~Env() = default;
    Env(Env& other) = delete;             // Remove the copy constructor
    void operator=(const Env&) = delete;  // Remove the copy assignment

    JNIEnv* operator->() { return _env; }

    operator JNIEnv*() { return _env; }

    // Class find_class(const std::string& name) {
    //     jclass jcls = _env->FindClass(name.c_str());
    //     if (jcls == nullptr) {
    //         throw std::runtime_error("Failed to find class: " + name);
    //     }
    //     Class cls(jcls);
    //     _env->DeleteLocalRef(jcls);
    //     return cls;
    // }

  private:
    JNIEnv* _env = nullptr;
};

class Runner {
  public:
    // Delete copy constructor and assignment
    Runner(const Runner&) = delete;
    Runner& operator=(const Runner&) = delete;

    static Runner& get() {
        static Runner instance;
        return instance;
    }

    virtual ~Runner() {
        {
            std::unique_lock<std::mutex> lock(_mutex);
            _stop = true;
            _cv.notify_one();
        }
        _thread.join();
    }

    void enqueue(std::function<void()> func) {
        {
            std::unique_lock<std::mutex> lock(_mutex);
            _queue.push(std::move(func));
            lock.unlock();
            _cv.notify_one();
        }
    }

  protected:
    void thread_func() {
        VM::attach();
        while (true) {
            std::function<void()> func;
            {
                std::unique_lock<std::mutex> lock(_mutex);
                _cv.wait(lock, [this] { return _stop || !_queue.empty(); });
                if (_stop && _queue.empty()) {
                    break;
                }
                func = std::move(_queue.front());
                _queue.pop();
            }
            func();
        }
        VM::detach();
    }

  private:
    // Move constructor to private
    Runner() : _stop(false) { _thread = std::thread(&Runner::thread_func, this); }

    std::thread _thread;
    std::mutex _mutex;
    std::condition_variable _cv;
    std::queue<std::function<void()>> _queue;
    bool _stop;
};

template <template <typename> class RefType, typename JniType = jobject>
struct ObjectComparator {

    bool operator()(const Object<RefType, JniType>& lhs, const Object<RefType, JniType>& rhs) const {
        // Handle null object comparisons
        if (!lhs && !rhs) {
            return false;  // Both are null, considered equal
        }
        if (!lhs) {
            return true;  // lhs is null, rhs is not, lhs < rhs
        }
        if (!rhs) {
            return false;  // rhs is null, lhs is not, lhs > rhs
        }

        JNIEnv* env = VM::env();

        // Access the underlying jobject handles from Object instances
        JniType lhsObject = lhs.get();
        JniType rhsObject = rhs.get();

        // Check if both jobject handles refer to the same object
        if (env->IsSameObject(lhsObject, rhsObject)) {
            return false;  // Both objects are the same
        }

        jmethodID method_hashCode = Registrar::get().get_method("java/lang/Object", "hashCode");
        jint lhsHashCode = env->CallIntMethod(lhsObject, method_hashCode);
        jint rhsHashCode = env->CallIntMethod(rhsObject, method_hashCode);

        if (lhsHashCode != rhsHashCode) {
            return lhsHashCode < rhsHashCode;  // Use hash code for initial comparison
        }

        // Use a direct pointer comparison as a fallback for objects with identical hash codes
        return lhsObject < rhsObject;  // This comparison is consistent within the same execution
    }
};
}  // namespace SimpleJNI
