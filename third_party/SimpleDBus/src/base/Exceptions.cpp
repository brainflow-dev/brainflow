#include <simpledbus/base/Exceptions.h>

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

}  // namespace Exception

}  // namespace SimpleDBus
