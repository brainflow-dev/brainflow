#pragma once

#include "UsbHelperImpl.h"

namespace SimpleBLE {
namespace Dongl {
namespace USB {

class UsbHelperNull : public UsbHelperImpl {
  public:
    UsbHelperNull(const std::string& device_path);
    ~UsbHelperNull();

    void tx(const kvn::bytearray& data);
    void set_rx_callback(std::function<void(const kvn::bytearray&)> callback);

    static std::vector<std::string> get_dongl_devices();
};

}  // namespace USB
}  // namespace Dongl
}  // namespace SimpleBLE