#pragma once

/*
 * Note: Only use this class within the SimpleDBus, as some internal name choices might
 * generate conflicts with other libraries.
 */

#include <mutex>
#include <string>

namespace SimpleDBus {

class Logger {
  public:
    typedef enum {
        NONE = 0,
        FATAL,
        ERROR,
        WARN,
        INFO,
#pragma push_macro("DEBUG")
#undef DEBUG
        DEBUG,
#pragma pop_macro("DEBUG")
        VERBOSE_0,
        VERBOSE_1,
        VERBOSE_2,
        VERBOSE_3,  // Used for tracking the creation/destruction of BlueZ abstractions.
    } LogLevel;

    static Logger* get();

    void log(LogLevel level, const char* file, const char* function, unsigned int line, const char* format, ...);
    void set_level(LogLevel level);

  private:
    Logger();
    ~Logger();
    Logger(Logger& other) = delete;          // Remove the copy constructor
    void operator=(const Logger&) = delete;  // Remove the copy assignment

    void print_log(std::string message);

    LogLevel _log_level;
    std::mutex _mutex;

    static std::string string_format(const char* format, ...);
    static std::string string_format(const char* format, va_list vlist);
    static std::string parse_function_signature(const char* function);
    static std::string parse_file_path(const char* file);
};

}  // namespace SimpleDBus

#define VLOG_F(level, ...) SimpleDBus::Logger::get()->log(level, __FILE__, __PRETTY_FUNCTION__, __LINE__, __VA_ARGS__)
#define LOG_F(level, ...) VLOG_F(SimpleDBus::Logger::LogLevel::level, __VA_ARGS__)
#define LOG_LEVEL_SET(level) SimpleDBus::Logger::get()->set_level(level)
