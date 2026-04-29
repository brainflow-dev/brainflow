#pragma once

#include <cstdint>
#include <functional>
#include <mutex>
#include <string>

#include <simpleble/export.h>

namespace SimpleBLE {

namespace Logging {

enum Level : int {
    None = 0,
    Fatal,
    Error,
    Warn,
    Info,
    Debug,
    Verbose,
};

// clang-format off
using Callback = std::function<void(
    Level,
    const std::string& module,
    const std::string& file,
    uint32_t line,
    const std::string& function,
    const std::string& message)>;
// clang-format on

class SIMPLEBLE_EXPORT Logger {
  public:
    static Logger* get();

    void set_level(Level level);
    Level get_level();

    void set_callback(Callback callback);
    bool has_callback();

    void log_default_stdout();
    void log_default_file();
    void log_default_file(const std::string path);

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

    static std::string level_to_str(Level level);

    Level level_{Level::Info};
    Callback callback_{nullptr};
    std::recursive_mutex mutex_;
};

}  // namespace Logging

}  // namespace SimpleBLE
