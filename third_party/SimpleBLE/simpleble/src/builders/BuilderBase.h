#pragma once

#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>

namespace SimpleBLE::Factory {

/**
 * Generic builder class for PIMP classes that have an "internal_" member variable.
 *
 * Normally, the base class's non-default constructor will not exist or will be
 * protected, so that the user cannot directly initialize the class (i.e. set
 * the "internal_" member variable to non-null).
 *
 * Additionally, this builder class allows for the construction of pointer-to
 * object directly in the constructor.
 *
 * It is possible to specialize this class for classes that are constructed
 * differently.
 */
template <typename T>
struct Builder : public T {
    using T::internal_;
    using PtrT = decltype(Builder::internal_);

    Builder() { this->internal_ = std::make_shared<InternalT>(); }
    using InternalT = typename std::remove_reference<decltype(*(Builder::internal_))>::type;

    Builder(PtrT&& internal) { this->internal_ = std::move(internal); }

    template <typename dPtr>
    Builder(dPtr&& internal) {
        this->internal_ = internal;
    }

    template <typename... ARG>
    Builder(ARG&&... args) {
        using InternalT = typename std::remove_reference<decltype(*(Builder::internal_))>::type;
        this->internal_ = std::make_shared<InternalT>(std::forward<ARG>(args)...);
    }
};

/**
 * Helper class to deduce the return type of the build function.
 */
template <typename ARG>
struct BuildDeduce {
    template <typename T>
    operator T() && {
        return Builder<T>(std::move(arg));
    }

    ARG arg;
};

struct BuildDeduce0 {
    template <typename T>
    operator T() && {
        return Builder<T>();
    }
};

/**
 * Build a new object which is not directly constructible.
 *
 * The target class must have a default constructor and must have a protected
 * "internal_" member variable.
 */
template <typename ARG>
auto build(ARG x) {
    return BuildDeduce<ARG>{std::move(x)};
}

/**
 * Build a new object whose "internal_" type can be default-constructed.
 *
 * For example, for Adapter, calling `Adapter a = Factory::build();` is equivalent to
 * `Adapter a = Build<Adapter>(std::make_shared<AdapterBase>());`.
 */
template <typename = void>
auto build() {
    return BuildDeduce0{};
}

}  // namespace SimpleBLE::Factory
