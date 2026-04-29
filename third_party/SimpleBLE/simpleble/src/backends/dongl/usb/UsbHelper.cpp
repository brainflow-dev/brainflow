#include "UsbHelper.h"

#if SIMPLEBLE_BACKEND_LINUX
#include "UsbHelperLinux.h"
#elif SIMPLEBLE_BACKEND_WINDOWS
#include "UsbHelperWindows.h"
#elif SIMPLEBLE_BACKEND_MACOS
#include "UsbHelperApple.h"
#else
#include "UsbHelperNull.h"
#endif

namespace SimpleBLE {
namespace Dongl {
namespace USB {

UsbHelper::UsbHelper(const std::string& device_path) : _impl(nullptr) {
#if SIMPLEBLE_BACKEND_LINUX
    _impl = std::make_unique<UsbHelperLinux>(device_path);
#elif SIMPLEBLE_BACKEND_WINDOWS
    _impl = std::make_unique<UsbHelperWindows>(device_path);
#elif SIMPLEBLE_BACKEND_MACOS
    _impl = std::make_unique<UsbHelperApple>(device_path);
#else
    _impl = std::make_unique<UsbHelperNull>(device_path);
#endif
}

UsbHelper::~UsbHelper() = default;

void UsbHelper::tx(const kvn::bytearray& data) {
    _impl->tx(data);
}

void UsbHelper::set_rx_callback(std::function<void(const kvn::bytearray&)> callback) {
    _impl->set_rx_callback(callback);
}

std::vector<std::string> UsbHelper::get_dongl_devices() {
#if SIMPLEBLE_BACKEND_LINUX
    return UsbHelperLinux::get_dongl_devices();
#elif SIMPLEBLE_BACKEND_WINDOWS
    return UsbHelperWindows::get_dongl_devices();
#elif SIMPLEBLE_BACKEND_MACOS
    return UsbHelperApple::get_dongl_devices();
#else
    return UsbHelperNull::get_dongl_devices();
#endif
}

}  // namespace USB
}  // namespace Dongl
}  // namespace SimpleBLE