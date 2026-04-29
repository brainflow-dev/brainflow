#pragma once
#include <chrono>

//clang-format off
namespace SimpleBLE {
/**
 * @namespace SimpleBLE::Config
 * @brief Configuration options for SimpleBLE.
 *
 * @note All configuration values must be set prior to any other interaction with a SimpleBLE component.
 *       Changes made after an adapter has been retrieved may not take effect or could lead to undefined behavior.
 */
namespace Config {
namespace SimpleBluez {
    extern bool use_legacy_bluez_backend;
    extern bool use_system_bus; // NOTE: This is only available in the new Bluez backend.
    extern std::chrono::steady_clock::duration connection_timeout;
    extern std::chrono::steady_clock::duration disconnection_timeout;

    static void reset() {
        use_legacy_bluez_backend = true;
        use_system_bus = true;
        connection_timeout = std::chrono::seconds(2);
        disconnection_timeout = std::chrono::seconds(1);
    }
}  // namespace SimpleBluez

namespace WinRT {
    extern bool experimental_use_own_mta_apartment;
    extern bool experimental_reinitialize_winrt_apartment_on_main_thread;

    static void reset() {
        experimental_use_own_mta_apartment = true;
        experimental_reinitialize_winrt_apartment_on_main_thread = false;
    }
}  // namespace WinRT

namespace CoreBluetooth {
    static void reset() {}
}  // namespace CoreBluetooth

namespace Android {
    enum class ConnectionPriorityRequest { DISABLED = -1, BALANCED = 0, HIGH = 1, LOW_POWER = 2, DCK = 3 };

    extern ConnectionPriorityRequest connection_priority_request;

    static void reset() { connection_priority_request = ConnectionPriorityRequest::DISABLED; }
}  // namespace Android

namespace Dongl {
    extern bool use_dongl_backend;
    static void reset() { use_dongl_backend = false; }
}  // namespace Dongl

namespace Base {
    static void reset_all() {
        SimpleBluez::reset();
        WinRT::reset();
        CoreBluetooth::reset();
        Android::reset();
    }
}  // namespace Base
}  // namespace Config
}  // namespace SimpleBLE
//clang-format on