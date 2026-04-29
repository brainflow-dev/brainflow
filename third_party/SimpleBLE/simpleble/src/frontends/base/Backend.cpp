#include <vector>

#include <simpleble/Adapter.h>
#include <simpleble/Config.h>
#include <simpleble/Exceptions.h>

#include "BackendBase.h"
#include "BuildVec.h"
#include "CommonUtils.h"

#include "Backend.h"

using namespace SimpleBLE;

namespace SimpleBLE {

static std::shared_ptr<BackendBase> _get_enabled_backend() {
    using BackendPtr = std::shared_ptr<BackendBase>(void);

    if (Config::Dongl::use_dongl_backend) {
        extern BackendPtr BACKEND_DONGL;
        return BACKEND_DONGL();
    }

    if constexpr (SIMPLEBLE_BACKEND_LINUX) {
        extern BackendPtr BACKEND_LINUX;
        extern BackendPtr BACKEND_LINUX_LEGACY;

        if (Config::SimpleBluez::use_legacy_bluez_backend) {
            return BACKEND_LINUX_LEGACY();
        } else {
            return BACKEND_LINUX();
        }
    } else if constexpr (SIMPLEBLE_BACKEND_WINDOWS) {
        extern BackendPtr BACKEND_WINDOWS;
        return BACKEND_WINDOWS();
    } else if constexpr (SIMPLEBLE_BACKEND_ANDROID) {
        extern BackendPtr BACKEND_ANDROID;
        return BACKEND_ANDROID();
    } else if constexpr (SIMPLEBLE_BACKEND_MACOS) {
        extern BackendPtr BACKEND_MACOS;
        return BACKEND_MACOS();
    } else if constexpr (SIMPLEBLE_BACKEND_IOS) {
        extern BackendPtr BACKEND_MACOS;
        return BACKEND_MACOS();
    } else if constexpr (SIMPLEBLE_BACKEND_PLAIN) {
        extern BackendPtr BACKEND_PLAIN;
        return BACKEND_PLAIN();
    }

    throw Exception::NotInitialized();
}

Backend get_enabled_backend() { return Factory::build(_get_enabled_backend()); }

// NOTE: in the future, this can return multiple backends
static SharedPtrVector<BackendBase> _get_backends() {
    SharedPtrVector<BackendBase> backends = {_get_enabled_backend()};
    return backends;
}

}  // namespace SimpleBLE

std::vector<Backend> Backend::get_backends() { return Factory::vector(_get_backends()); }

bool Backend::initialized() const { return internal_ != nullptr; }

BackendBase* Backend::operator->() {
    if (!initialized()) {
        throw Exception::NotInitialized();
    }
    return internal_.get();
}

const BackendBase* Backend::operator->() const {
    if (!initialized()) {
        throw Exception::NotInitialized();
    }
    return internal_.get();
}

std::vector<Adapter> Backend::get_adapters() { return Factory::vector((*this)->get_adapters()); }

bool Backend::bluetooth_enabled() { return (*this)->bluetooth_enabled(); }

std::optional<Backend> Backend::first_bluetooth_enabled() {
    for (auto& backend : get_backends()) {
        if (backend->bluetooth_enabled()) {
            return backend;
        }
    }
    return std::nullopt;
}

std::string Backend::name() const noexcept { return (*this)->name(); }
