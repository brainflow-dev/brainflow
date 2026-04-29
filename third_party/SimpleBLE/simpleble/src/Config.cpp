#include <simpleble/Config.h>

namespace SimpleBLE {
namespace Config {
    namespace SimpleBluez {
        bool use_legacy_bluez_backend = true;
        bool use_system_bus = true;
        std::chrono::steady_clock::duration connection_timeout = std::chrono::seconds(2);
        std::chrono::steady_clock::duration disconnection_timeout = std::chrono::seconds(1);
    }  // namespace SimpleBluez

    namespace WinRT {
        bool experimental_use_own_mta_apartment = true;
        bool experimental_reinitialize_winrt_apartment_on_main_thread = false;
    }  // namespace WinRT

    namespace Android {
        ConnectionPriorityRequest connection_priority_request = ConnectionPriorityRequest::DISABLED;
    }  // namespace Android

    namespace Dongl {
        bool use_dongl_backend = false;
    }  // namespace Dongl

}  // namespace Config
}  // namespace SimpleBLE