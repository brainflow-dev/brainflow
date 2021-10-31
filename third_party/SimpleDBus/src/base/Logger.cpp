#include <simpledbus/base/Logger.h>

#include <cstdarg>
#include <cstdlib>
#include <iostream>

using namespace SimpleDBus;

static const char* log_level_strings[] = {"NONE",  "FATAL",   "ERROR",   "WARN",    "INFO",
                                          "DEBUG", "VERBOSE_0", "VERBOSE_1", "VERBOSE_2", "VERBOSE_3"};

#ifndef SIMPLEDBUS_LOG_LEVEL
#define SIMPLEDBUS_LOG_LEVEL FATAL
#endif

Logger::Logger() : _log_level(LogLevel::SIMPLEDBUS_LOG_LEVEL) {}

Logger::~Logger() {}

Logger* Logger::get() {
    static std::mutex get_mutex;       // Static mutex to ensure thread safety when accessing the logger
    std::scoped_lock lock(get_mutex);  // Unlock the mutex on function return
    static Logger instance;            // Static instance of the logger to ensure proper lifecycle management
    return &instance;
}

void Logger::print_log(std::string message) { std::cerr << message << std::endl; }

void Logger::log(LogLevel level, const char* file, const char* function, unsigned int line, const char* format, ...) {
    std::scoped_lock lock(_mutex);
    if (level > _log_level) return;

    va_list vlist;
    va_start(vlist, format);
    std::string user_message = Logger::string_format(format, vlist);
    va_end(vlist);

    std::string function_signature = parse_function_signature(function);
    std::string filename = parse_file_path(file);

    std::string log_message = Logger::string_format("[%9s] %s (%s:%u) %s", log_level_strings[level], filename.c_str(),
                                                    function_signature.c_str(), line, user_message.c_str());
    print_log(log_message);
}

void Logger::set_level(LogLevel level) {
    std::scoped_lock lock(_mutex);
    _log_level = level;
}

std::string Logger::string_format(const char* format, ...) {
    std::string text = "";
    va_list vlist;
    va_start(vlist, format);
    text = Logger::string_format(format, vlist);
    va_end(vlist);
    return text;
}

std::string Logger::string_format(const char* format, va_list vlist) {
    std::string text = "";
    char* text_buffer = nullptr;

    int result = vasprintf(&text_buffer, format, vlist);

    if (result >= 0) {
        text = std::string(text_buffer);
        free(text_buffer);
    } else {
        // An error has happened with vasprintf.
        printf("Error during message generation. Format was: '%s'", format);
        // Abort program execution as we might have entered some invalid state.
        abort();
    }
    return text;
}

std::string Logger::parse_function_signature(const char* function) {
    // NOTE: This function will have trouble with more function signatures having types
    // that contain spaces or
    std::string function_signature(function);

    std::size_t name_start = function_signature.find_first_of(" ") + 1;
    std::size_t name_end = function_signature.find_first_of("(");

    return function_signature.substr(name_start, name_end - name_start);
}

std::string Logger::parse_file_path(const char* file) {
    std::string file_path(file);
    std::size_t name_start = file_path.find_last_of("/") + 1;
    return file_path.substr(name_start);
}
