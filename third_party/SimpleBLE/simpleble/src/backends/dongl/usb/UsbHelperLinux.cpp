#include "UsbHelperLinux.h"

namespace SimpleBLE {
namespace Dongl {
namespace USB {

UsbHelperLinux::UsbHelperLinux(const std::string& device_path) : UsbHelperImpl(device_path) {}

UsbHelperLinux::~UsbHelperLinux() = default;

void UsbHelperLinux::tx(const kvn::bytearray& data) {}

void UsbHelperLinux::set_rx_callback(std::function<void(const kvn::bytearray&)> callback) {
    _rx_callback.load(callback);
}

std::vector<std::string> UsbHelperLinux::get_dongl_devices() { return {}; }

}  // namespace USB
}  // namespace Dongl
}  // namespace SimpleBLE
