#pragma once

#include <vector>
#include <string>
#include <memory>

#include <kvn/kvn_safe_callback.hpp>
#include <kvn/kvn_bytearray.h>

namespace SimpleBLE {
namespace Dongl {
namespace USB {

class UsbHelperImpl;

class UsbHelper {
  public:
    UsbHelper(const std::string& device_path);
    ~UsbHelper();

    void tx(const kvn::bytearray& data);
    void set_rx_callback(std::function<void(const kvn::bytearray&)> callback);

    static std::vector<std::string> get_dongl_devices();

  protected:
    std::unique_ptr<UsbHelperImpl> _impl;
};

}  // namespace USB
}  // namespace Dongl
}  // namespace SimpleBLE
