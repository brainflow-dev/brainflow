#pragma once

#include <memory>

#include "BackendBase.h"

namespace SimpleBLE {

/**
 * Convenience CRTP class to have a Backend that is a singleton.
 *
 * REQUIRES: T must be default-constructible.
 */
template <typename T>
class BackendSingleton : public BackendBase, public std::enable_shared_from_this<T> {
  public:
    static std::shared_ptr<T> get() {
        static T instance(buildToken{});
        return std::shared_ptr<T>(&instance, [](T*) {});
    }

    BackendSingleton(const BackendSingleton& other) = delete;  // Remove the copy constructor
    T& operator=(const T&) = delete;                           // Remove the copy assignment

  protected:
    // Derived classes should have a constructor that takes a buildToken to
    // ensure that the class is only constructed through the get() method.
    struct buildToken {};

    BackendSingleton() = default;
};

}  // namespace SimpleBLE
