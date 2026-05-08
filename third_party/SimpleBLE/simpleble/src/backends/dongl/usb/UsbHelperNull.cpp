#include "UsbHelperNull.h"

namespace SimpleBLE {
namespace Dongl {
namespace USB {

UsbHelperNull::UsbHelperNull(const std::string& device_path) : UsbHelperImpl(device_path) {}

UsbHelperNull::~UsbHelperNull() = default;

void UsbHelperNull::tx(const kvn::bytearray& data) {}

void UsbHelperNull::set_rx_callback(std::function<void(const kvn::bytearray&)> callback) {
    _rx_callback.load(callback);
}

std::vector<std::string> UsbHelperNull::get_dongl_devices() { return {}; }

}  // namespace USB
}  // namespace Dongl
}  // namespace SimpleBLE
