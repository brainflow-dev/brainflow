#include "simpleble/Logging.h"

#include <fmt/core.h>
#include <mutex>

using namespace SimpleBLE::Logging;

Logger* Logger::get() {
    static std::mutex get_mutex;       // Static mutex to ensure thread safety when accessing the logger
    std::scoped_lock lock(get_mutex);  // Unlock the mutex on function return
    static Logger instance;            // Static instance of the logger to ensure proper lifecycle management
    return &instance;
}

Logger::Logger() {
    set_callback([](Level level, const std::string& module, const std::string& file, uint32_t line,
                    const std::string& function, const std::string& message) {
        std::string level_string;
        switch (level) {
            case Level::VERBOSE:
                level_string = "VERBOSE";
                break;
            case Level::DEBUG:
                level_string = "DEBUG";
                break;
            case Level::INFO:
                level_string = "INFO";
                break;
            case Level::WARN:
                level_string = "WARNING";
                break;
            case Level::ERROR:
                level_string = "ERROR";
                break;
            case Level::FATAL:
                level_string = "FATAL";
                break;
            case Level::NONE:
                return;
        }

        fmt::print("{} [{}] {}:{} {} - {}\n", level_string, module, file, line, function, message);
    });
}

Logger::~Logger() {}

void Logger::set_level(Level level) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    level_ = level;
}

Level Logger::get_level() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    return level_;
}

void Logger::set_callback(Callback callback) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    callback_ = callback;
}

bool Logger::has_callback() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    return callback_ != nullptr;
}

void Logger::log(Level level, const std::string& module, const std::string& file, uint32_t line,
                 const std::string& function, const std::string& message) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    if (level <= level_ && callback_ != nullptr) {
        try {
            callback_(level, module, file, line, function, message);
        } catch (...) {
            // Clearly, if the logging callback throws an exception, we should not crash.
        }
    }
}

#if defined(__linux__) || defined(__APPLE__)
// For Linux and MacOS, implement a strong definition of the logfwd receive library.
#include "external/logfwd.hpp"

// clang-format off
void logfwd::receive(
    logfwd::level level,
    const std::string& module,
    const std::string& file,
    uint32_t line,
    const std::string& function,
    const std::string& message) {
    
    // Forward logs from internal modules into the SimpleBLE logger.
    Logger::get()->log(
        static_cast<Level>(level),
        fmt::format("SimpleBLE->{}", module),
        file,
        line,
        function,
        message);
}
// clang-format on

#endif
