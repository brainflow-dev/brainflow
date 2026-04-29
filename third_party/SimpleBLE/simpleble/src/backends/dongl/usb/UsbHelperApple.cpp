#include "UsbHelperApple.h"

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOReturn.h>
#include <IOKit/serial/IOSerialKeys.h>
#include <IOKit/usb/IOUSBLib.h>
#include <IOKit/serial/ioss.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <fmt/core.h>

namespace SimpleBLE {
namespace Dongl {
namespace USB {

template <typename T, typename Deleter>
class ScopedResource {
  public:
    ScopedResource(T resource = nullptr) : resource_(resource) {}
    ~ScopedResource() {
        if (resource_) deleter_(resource_);
    }

    T get() const { return resource_; }
    T operator->() const { return resource_; }
    operator bool() const { return resource_ != nullptr; }

  private:
    T resource_;
    Deleter deleter_;
};

struct CFReleaser {
    void operator()(CFTypeRef obj) const { CFRelease(obj); }
};

struct IOReleaser {
    void operator()(io_object_t obj) const { IOObjectRelease(obj); }
};

using ScopedCFRef = ScopedResource<CFTypeRef, CFReleaser>;
using ScopedIOObject = ScopedResource<io_object_t, IOReleaser>;

UsbHelperApple::UsbHelperApple(const std::string& device_path) : UsbHelperImpl(device_path) {
        // Open and configure the serial port
        if (!_open_serial_port()) {
            throw std::runtime_error("Failed to open serial port: " + _device_path);
        }

        _running = true;
        _thread = std::thread(&UsbHelperApple::_run, this);
}

UsbHelperApple::~UsbHelperApple() {
    _running = false;
    if (_thread.joinable()) {
        _thread.join();
    }
    _close_serial_port();
}

void UsbHelperApple::tx(const kvn::bytearray& data) {
    const ssize_t written = write(_serial_fd, data.data(), data.size());
    if (written != data.size()) {
        throw std::runtime_error("Failed to write to serial port: " + std::string(strerror(errno)));
    }
}

void UsbHelperApple::set_rx_callback(std::function<void(const kvn::bytearray&)> callback) {
    _rx_callback.load(callback);
}

std::vector<std::string> UsbHelperApple::get_dongl_devices() {
    std::vector<std::string> dongle_devices;

    io_iterator_t raw_iterator = 0;
    if (IOServiceGetMatchingServices(kIOMainPortDefault, IOServiceMatching(kIOSerialBSDServiceValue), &raw_iterator) !=
        kIOReturnSuccess) {
        return {};
    }
    ScopedIOObject iterator(raw_iterator);

    io_service_t raw_service;
    while ((raw_service = IOIteratorNext(iterator.get()))) {
        ScopedIOObject service_guard(raw_service);

        // Get the callout device raw_path (/dev/cu.*)
        char raw_path[PATH_MAX];
        ScopedCFRef pathCF((CFStringRef)IORegistryEntryCreateCFProperty(raw_service, CFSTR(kIOCalloutDeviceKey),
                                                                        kCFAllocatorDefault, 0));
        if (!pathCF) {
            continue;
        }

        if (!CFStringGetCString((CFStringRef)pathCF.get(), raw_path, sizeof(raw_path), kCFStringEncodingUTF8)) {
            continue;
        }

        std::string path(raw_path);

        //fmt::print("Checking device path: {}\n", path);

        // Search up parents for idVendor and idProduct
        ScopedCFRef vidCF((CFNumberRef)IORegistryEntrySearchCFProperty(
            raw_service, kIOServicePlane, CFSTR(kUSBVendorID), kCFAllocatorDefault,
            kIORegistryIterateParents | kIORegistryIterateRecursively));
        ScopedCFRef pidCF((CFNumberRef)IORegistryEntrySearchCFProperty(
            raw_service, kIOServicePlane, CFSTR(kUSBProductID), kCFAllocatorDefault,
            kIORegistryIterateParents | kIORegistryIterateRecursively));

        if (!vidCF || !pidCF) {
            continue;
        }

        uint16_t vid, pid;
        CFNumberGetValue((CFNumberRef)vidCF.get(), kCFNumberSInt16Type, &vid);
        CFNumberGetValue((CFNumberRef)pidCF.get(), kCFNumberSInt16Type, &pid);

        //fmt::print("VID/PID: 0x{:x}/0x{:x}\n", vid, pid);

        if (vid == UsbHelperImpl::DONGL_VENDOR_ID && pid == UsbHelperImpl::DONGL_PRODUCT_ID) {
            dongle_devices.push_back(path);
        }
    }
    return dongle_devices;
}

bool UsbHelperApple::_open_serial_port() {
    // Open the serial port in non-blocking mode
    _serial_fd = open(_device_path.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (_serial_fd < 0) {
        std::cerr << "Failed to open serial port " << _device_path << ": " << strerror(errno) << std::endl;
        return false;
    }

    // Configure the serial port
    _configure_serial_port();

    //std::cout << "Successfully opened serial port: " << _device_path << std::endl;
    return true;
}

void UsbHelperApple::_configure_serial_port() {
    struct termios tty;

    // Get current serial port settings
    if (tcgetattr(_serial_fd, &tty) != 0) {
        throw std::runtime_error("Failed to get serial port attributes: " + std::string(strerror(errno)));
    }

    // Configure serial port settings
    tty.c_cflag &= ~PARENB;         // No parity bit
    tty.c_cflag &= ~CSTOPB;         // One stop bit
    tty.c_cflag &= ~CSIZE;          // Clear data size bits
    tty.c_cflag |= CS8;             // 8 data bits
    tty.c_cflag &= ~CRTSCTS;        // Disable RTS/CTS hardware flow control
    tty.c_cflag |= CREAD | CLOCAL;  // Enable receiver, ignore modem control lines

    tty.c_lflag &= ~ICANON;  // Disable canonical mode
    tty.c_lflag &= ~ECHO;    // Disable echo
    tty.c_lflag &= ~ECHOE;   // Disable erasure echo
    tty.c_lflag &= ~ECHONL;  // Disable new-line echo
    tty.c_lflag &= ~ISIG;    // Disable interpretation of INTR, QUIT and SUSP

    tty.c_iflag &= ~(IXON | IXOFF | IXANY);                                       // Disable software flow control
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);  // Disable special handling

    tty.c_oflag &= ~OPOST;  // Disable output processing
    tty.c_oflag &= ~ONLCR;  // Disable conversion of newline to carriage return/line feed

    // Set minimum characters to read and timeout
    tty.c_cc[VMIN] = 0;   // Read doesn't block
    tty.c_cc[VTIME] = 0;  // 0.0 seconds read timeout

    // Apply the settings
    if (tcsetattr(_serial_fd, TCSANOW, &tty) != 0) {
        throw std::runtime_error("Failed to set serial port attributes: " + std::string(strerror(errno)));
    }

    speed_t speed = 1000000;  // Set 1 Mbps
    if (ioctl(_serial_fd, IOSSIOSPEED, &speed) == -1) {
        throw std::runtime_error("Failed to set serial port speed: " + std::string(strerror(errno)));
    }

    // Flush any existing data
    tcflush(_serial_fd, TCIOFLUSH);
}

void UsbHelperApple::_close_serial_port() {
    if (_serial_fd >= 0) {
        close(_serial_fd);
        _serial_fd = -1;
        //std::cout << "Closed serial port: " << _device_path << std::endl;
    }
}

void UsbHelperApple::_run() {
    const size_t BUFFER_SIZE = 256;
    char buffer[BUFFER_SIZE];

    fd_set read_fds;
    struct timeval timeout;

    //std::cout << "UsbHelperApple: _run() started with non-blocking reads" << std::endl;

    while (_running) {
        // Clear the file descriptor set
        FD_ZERO(&read_fds);
        FD_SET(_serial_fd, &read_fds);

        // Set timeout for select (100ms)
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;  // 100ms

        // Wait for data to be available (non-blocking)
        int ready = select(_serial_fd + 1, &read_fds, nullptr, nullptr, &timeout);

        if (ready > 0) {
            // Data is available to read
            if (FD_ISSET(_serial_fd, &read_fds)) {
                ssize_t bytes_read = read(_serial_fd, buffer, BUFFER_SIZE - 1);

                if (bytes_read > 0) {
                    kvn::bytearray data(buffer, bytes_read);
                    _rx_callback(data);
                } else if (bytes_read == 0) {
                    // End of file (device disconnected)
                    std::cerr << "Serial port disconnected" << std::endl;
                    break;
                } else {
                    // Error reading from serial port
                    if (errno != EAGAIN && errno != EWOULDBLOCK) {
                        std::cerr << "Error reading from serial port: " << strerror(errno) << std::endl;
                        break;
                    }
                }
            }
        } else if (ready == 0) {
            // Timeout - no data available, continue loop
            continue;
        } else {
            // Error in select
            if (errno != EINTR) {
                std::cerr << "Error in select: " << strerror(errno) << std::endl;
                break;
            }
        }
    }
}


}  // namespace USB
}  // namespace Dongl
}  // namespace SimpleBLE
