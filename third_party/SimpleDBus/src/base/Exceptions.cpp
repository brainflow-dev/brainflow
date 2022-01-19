#include <simpledbus/base/Exceptions.h>

#include <fmt/core.h>

namespace SimpleDBus {

namespace Exception {

NotInitialized::NotInitialized() {}

const char* NotInitialized::what() const noexcept { return "Object not initialized."; }

DBusException::DBusException(const std::string& err_name, const std::string& err_message) {
    _message = fmt::format("{}: {}", err_name, err_message);
}

const char* DBusException::what() const noexcept { return _message.c_str(); }

SendFailed::SendFailed(const std::string& err_name, const std::string& err_message, const std::string& msg_str) {
    _message = fmt::format("{}: {}\n{}", err_name, err_message, msg_str);
}

const char* SendFailed::what() const noexcept { return _message.c_str(); }

InterfaceNotFoundException::InterfaceNotFoundException(const std::string& path, const std::string& interface) {
    _message = fmt::format("Path {} does not contain interface {}", path, interface);
}

const char* InterfaceNotFoundException::what() const noexcept { return _message.c_str(); }

PathNotFoundException::PathNotFoundException(const std::string& path, const std::string& subpath) {
    _message = fmt::format("Path {} does not contain sub-path {}", path, subpath);
}

const char* PathNotFoundException::what() const noexcept { return _message.c_str(); }

}  // namespace Exception

}  // namespace SimpleDBus
