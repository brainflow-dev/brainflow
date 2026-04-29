#pragma once

#include <jni.h>

#include "VM.hpp"

namespace SimpleJNI {

template <typename T>
class GlobalRef {
  public:
    GlobalRef() = default;

    explicit GlobalRef(T obj) : _obj(obj ? static_cast<T>(VM::env()->NewGlobalRef(obj)) : nullptr) {}

    ~GlobalRef() {
        if (_obj && VM::is_jvm_alive()) VM::env()->DeleteGlobalRef(_obj);
    }

    // Copy constructor
    GlobalRef(const GlobalRef& other)
        : _obj(other._obj ? static_cast<T>(VM::env()->NewGlobalRef(other._obj)) : nullptr) {}

    // Copy assignment operator
    GlobalRef& operator=(const GlobalRef& other) {
        if (this != &other) {
            if (_obj) VM::env()->DeleteGlobalRef(_obj);
            _obj = other._obj ? static_cast<T>(VM::env()->NewGlobalRef(other._obj)) : nullptr;
        }
        return *this;
    }

    // Move constructor
    GlobalRef(GlobalRef&& other) noexcept : _obj(other._obj) { other._obj = nullptr; }

    // Move assignment operator
    GlobalRef& operator=(GlobalRef&& other) noexcept {
        if (this != &other) {
            if (_obj) VM::env()->DeleteGlobalRef(_obj);
            _obj = other._obj;
            other._obj = nullptr;
        }
        return *this;
    }

    T get() const { return _obj; }

  protected:
    T _obj = nullptr;
};

template <typename T>
class LocalRef {
  public:
    LocalRef() = default;

    explicit LocalRef(T obj) : _obj(obj ? static_cast<T>(VM::env()->NewLocalRef(obj)) : nullptr) {}

    ~LocalRef() {
        if (_obj) VM::env()->DeleteLocalRef(_obj);
    }

    // Copy constructor
    LocalRef(const LocalRef& other) : _obj(other._obj ? static_cast<T>(VM::env()->NewLocalRef(other._obj)) : nullptr) {}

    // Copy assignment operator
    LocalRef& operator=(const LocalRef& other) {
        if (this != &other) {
            if (_obj) VM::env()->DeleteLocalRef(_obj);
            _obj = other._obj ? static_cast<T>(VM::env()->NewLocalRef(other._obj)) : nullptr;
        }
        return *this;
    }

    // Move constructor
    LocalRef(LocalRef&& other) noexcept : _obj(other._obj) { other._obj = nullptr; }

    // Move assignment operator
    LocalRef& operator=(LocalRef&& other) noexcept {
        if (this != &other) {
            if (_obj) VM::env()->DeleteLocalRef(_obj);
            _obj = other._obj;
            other._obj = nullptr;
        }
        return *this;
    }

    T get() const { return _obj; }

  protected:
    T _obj = nullptr;
};

// A type for references intended to be returned to JNI
template <typename T>
class ReleasableLocalRef : public LocalRef<T> {
  public:
    using LocalRef<T>::LocalRef;
    T release() noexcept {
        T obj = this->_obj;
        this->_obj = nullptr;
        return obj;
    }
};

template <typename T>
class WeakRef {
  public:
    WeakRef() = default;

    explicit WeakRef(T obj) : _obj(obj ? static_cast<T>(VM::env()->NewWeakGlobalRef(obj)) : nullptr) {}

    ~WeakRef() {
        if (_obj && VM::is_jvm_alive()) VM::env()->DeleteWeakGlobalRef(_obj);
    }

    // Copy constructor
    WeakRef(const WeakRef& other)
        : _obj(other._obj ? static_cast<T>(VM::env()->NewWeakGlobalRef(other._obj)) : nullptr) {}

    // Copy assignment operator
    WeakRef& operator=(const WeakRef& other) {
        if (this != &other) {
            if (_obj) VM::env()->DeleteWeakGlobalRef(_obj);
            _obj = other._obj ? static_cast<T>(VM::env()->NewWeakGlobalRef(other._obj)) : nullptr;
        }
        return *this;
    }

    // Move constructor
    WeakRef(WeakRef&& other) noexcept : _obj(other._obj) { other._obj = nullptr; }

    // Move assignment operator
    WeakRef& operator=(WeakRef&& other) noexcept {
        if (this != &other) {
            if (_obj) VM::env()->DeleteWeakGlobalRef(_obj);
            _obj = other._obj;
            other._obj = nullptr;
        }
        return *this;
    }

    T get() const { return _obj; }

    bool is_valid() const { return _obj && !VM::env()->IsSameObject(_obj, nullptr); }

  protected:
    T _obj = nullptr;
};

}  // namespace SimpleJNI
