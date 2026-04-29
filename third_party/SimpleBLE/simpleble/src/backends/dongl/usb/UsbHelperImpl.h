#pragma once

#include <kvn/kvn_bytearray.h>
#include <kvn/kvn_safe_callback.hpp>
#include <string>

namespace SimpleBLE {
namespace Dongl {
namespace USB {

class UsbHelperImpl {
  public:
    UsbHelperImpl(const std::string& device_path) : _device_path(device_path) {}
    virtual ~UsbHelperImpl() = default;

    virtual void tx(const kvn::bytearray& data) = 0;
    virtual void set_rx_callback(std::function<void(const kvn::bytearray&)> callback) = 0;

    static const uint16_t DONGL_VENDOR_ID = 0x9999; // 0x0403 for legacy dongles
    static const uint16_t DONGL_PRODUCT_ID = 0x0001; // 0x6001 for legacy dongles

  protected:
    std::string _device_path;
    kvn::safe_callback<void(const kvn::bytearray&)> _rx_callback;
};

}  // namespace USB
}  // namespace Dongl
}  // namespace SimpleBLE