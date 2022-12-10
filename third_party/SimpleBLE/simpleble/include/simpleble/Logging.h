#pragma once

#include <cstdint>
#include <functional>
#include <mutex>
#include <string>

#include <simpleble/export.h>

namespace SimpleBLE {

namespace Logging {

enum Level : int {
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
using Callback = std::function<void(Level, const std::string&, const std::string&, uint32_t, const std::string&, const std::string&)>;
// clang-format on

class SIMPLEBLE_EXPORT Logger {
  public:
    static Logger* get();

    void set_level(Level level);
    Level get_level();

    void set_callback(Callback callback);
    bool has_callback();

    // clang-format off
    void log(
        Level level,
        const std::string& module,
        const std::string& file,
        uint32_t line,
        const std::string& function,
        const std::string& message);
    // clang-format on

  private:
    Logger();
    ~Logger();
    Logger(Logger& other) = delete;          // Remove copy constructor
    void operator=(const Logger&) = delete;  // Remove copy assignment

    Level level_{Level::INFO};
    Callback callback_{nullptr};
    std::recursive_mutex mutex_;
};

}  // namespace Logging

}  // namespace SimpleBLE
