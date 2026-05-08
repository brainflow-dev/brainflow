#pragma once

#include "UsbHelperImpl.h"

namespace SimpleBLE {
namespace Dongl {
namespace USB {

class UsbHelperLinux : public UsbHelperImpl {
  public:
    UsbHelperLinux(const std::string& device_path);
    ~UsbHelperLinux();

    void tx(const kvn::bytearray& data);
    void set_rx_callback(std::function<void(const kvn::bytearray&)> callback);

    static std::vector<std::string> get_dongl_devices();
};

}  // namespace USB
}  // namespace Dongl
}  // namespace SimpleBLE