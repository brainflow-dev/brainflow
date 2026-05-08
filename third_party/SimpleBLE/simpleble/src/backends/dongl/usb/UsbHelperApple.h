#pragma once

#include "UsbHelperImpl.h"
#include <thread>
#include <atomic>

namespace SimpleBLE {
namespace Dongl {
namespace USB {

class UsbHelperApple : public UsbHelperImpl {
  public:
    UsbHelperApple(const std::string& device_path);
    ~UsbHelperApple();

    void tx(const kvn::bytearray& data);
    void set_rx_callback(std::function<void(const kvn::bytearray&)> callback);

    static std::vector<std::string> get_dongl_devices();

  private:
    void _run();
    std::atomic_bool _running;
    std::thread _thread;

    int _serial_fd;
    bool _open_serial_port();
    void _close_serial_port();
    void _configure_serial_port();
};

}  // namespace USB
}  // namespace Dongl
}  // namespace SimpleBLE