#include "UsbHelperWindows.h"

namespace SimpleBLE {
namespace Dongl {
namespace USB {

UsbHelperWindows::UsbHelperWindows(const std::string& device_path) : UsbHelperImpl(device_path) {}

UsbHelperWindows::~UsbHelperWindows() = default;

void UsbHelperWindows::tx(const kvn::bytearray& data) {}

void UsbHelperWindows::set_rx_callback(std::function<void(const kvn::bytearray&)> callback) {
    _rx_callback.load(callback);
}

std::vector<std::string> UsbHelperWindows::get_dongl_devices() { return {}; }

}  // namespace USB
}  // namespace Dongl
}  // namespace SimpleBLE
