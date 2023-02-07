#include "simpleble/Logging.h"

#include <fmt/chrono.h>
#include <fmt/core.h>
#include <ctime>
#include <fstream>
#include <mutex>

using namespace SimpleBLE::Logging;

Logger* Logger::get() {
    static std::mutex get_mutex;       // Static mutex to ensure thread safety when accessing the logger
    std::scoped_lock lock(get_mutex);  // Unlock the mutex on function return
    static Logger instance;            // Static instance of the logger to ensure proper lifecycle management
    return &instance;
}

Logger::Logger() { log_default_stdout(); }

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

std::string Logger::level_to_str(Level level) {
    std::string level_str;
    switch (level) {
        case Level::Verbose:
            level_str = "VERBOSE";
            break;
        case Level::Debug:
            level_str = "DEBUG";
            break;
        case Level::Info:
            level_str = "INFO";
            break;
        case Level::Warn:
            level_str = "WARNING";
            break;
        case Level::Error:
            level_str = "ERROR";
            break;
        case Level::Fatal:
            level_str = "FATAL";
            break;
        case Level::None:
            level_str = "NONE";
            break;
    }
    return level_str;
}

void Logger::log_default_stdout() {
    set_callback([](Level level, const std::string& module, const std::string& file, uint32_t line,
                    const std::string& function, const std::string& message) {
        std::string level_str = level_to_str(level);
        fmt::print("[{}] {}: {}:{} in {}: {}\n", level_str, module, file, line, function, message);
    });
}

void Logger::log_default_file() {
    std::time_t t = std::time(nullptr);
    std::string date_time_str = fmt::format("{:%Y-%m-%d_%H-%M-%S}", fmt::localtime(t));
    std::string filename = fmt::format("simpleble_{}", date_time_str);
    log_default_file(filename);
}

void Logger::log_default_file(const std::string path) {
    set_callback([=](Level level, const std::string& module, const std::string& file, uint32_t line,
                     const std::string& function, const std::string& message) {
        std::string level_str = level_to_str(level);
        std::string log_message = fmt::format("[{}] {}: {}:{} in {}: {}\n", level_str, module, file, line, function,
                                              message);

        std::ofstream outfile;
        outfile.open(path, std::ios_base::app);  // open the file in append mode
        outfile << log_message;
        outfile.close();
    });
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
