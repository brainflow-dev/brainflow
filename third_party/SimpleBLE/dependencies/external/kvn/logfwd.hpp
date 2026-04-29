#ifndef LOGFWD_HPP
#define LOGFWD_HPP

#include <string>

namespace logfwd {

enum level : int {
    NONE = 0,
    FATAL,
    ERROR,
    WARN,
    INFO,
#pragma push_macro("DEBUG")
#undef DEBUG
    DEBUG,
#pragma pop_macro("DEBUG")
    VERBOSE,
};

// clang-format off

void receive(
    level level,
    const std::string& module,
    const std::string& file,
    uint32_t line,
    const std::string& function,
    const std::string& message);

// clang-format on

}  // namespace logfwd

#endif  // LOGFWD_HPP
