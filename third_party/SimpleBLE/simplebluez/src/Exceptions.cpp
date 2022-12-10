#include <simplebluez/Exceptions.h>

#include <fmt/core.h>

namespace SimpleBluez {

namespace Exception {

ServiceNotFoundException::ServiceNotFoundException(const std::string& service) {
    _message = fmt::format("Service {} not found.", service);
}

const char* ServiceNotFoundException::what() const noexcept { return _message.c_str(); }

CharacteristicNotFoundException::CharacteristicNotFoundException(const std::string& characteristic) {
    _message = fmt::format("Characteristic {} not found.", characteristic);
}

const char* CharacteristicNotFoundException::what() const noexcept { return _message.c_str(); }

DescriptorNotFoundException::DescriptorNotFoundException(const std::string& descriptor) {
    _message = fmt::format("Descriptor {} not found.", descriptor);
}

const char* DescriptorNotFoundException::what() const noexcept { return _message.c_str(); }

}  // namespace Exception

}  // namespace SimpleBluez
