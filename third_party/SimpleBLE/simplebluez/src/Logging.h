#pragma once

#include <fmt/core.h>
#include <cstdint>
#include <string>

namespace SimpleBluez {

// clang-format off

void log_fatal(const std::string& file, uint32_t line, const std::string& function, const std::string& message);
void log_error(const std::string& file, uint32_t line, const std::string& function, const std::string& message);
void log_warn(const std::string& file, uint32_t line, const std::string& function, const std::string& message);
void log_info(const std::string& file, uint32_t line, const std::string& function, const std::string& message);
void log_debug(const std::string& file, uint32_t line, const std::string& function, const std::string& message);
void log_verbose(const std::string& file, uint32_t line, const std::string& function, const std::string& message);

template <typename T, typename... Args>
void log_fatal(const std::string& file, uint32_t line, const std::string& function, const T& t, const Args&... args) {
    log_fatal(file, line, function, fmt::format(t, args...));
}

template <typename T, typename... Args>
void log_error(const std::string& file, uint32_t line, const std::string& function, const T& t, const Args&... args) {
    log_error(file, line, function, fmt::format(t, args...));
}

template <typename T, typename... Args>
void log_warn(const std::string& file, uint32_t line, const std::string& function, const T& t, const Args&... args) {
    log_warn(file, line, function, fmt::format(t, args...));
}

template <typename T, typename... Args>
void log_info(const std::string& file, uint32_t line, const std::string& function, const T& t, const Args&... args) {
    log_info(file, line, function, fmt::format(t, args...));
}

template <typename T, typename... Args>
void log_debug(const std::string& file, uint32_t line, const std::string& function, const T& t, const Args&... args) {
    log_debug(file, line, function, fmt::format(t, args...));
}

template <typename T, typename... Args>
void log_verbose(const std::string& file, uint32_t line, const std::string& function, const T& t, const Args&... args) {
    log_verbose(file, line, function, fmt::format(t, args...));
}

// clang-format on

}  // namespace SimpleBluez

#define LOG_FATAL(...) SimpleBluez::log_fatal(__FILE__, __LINE__, __PRETTY_FUNCTION__, __VA_ARGS__)
#define LOG_ERROR(...) SimpleBluez::log_error(__FILE__, __LINE__, __PRETTY_FUNCTION__, __VA_ARGS__)
#define LOG_WARN(...) SimpleBluez::log_warn(__FILE__, __LINE__, __PRETTY_FUNCTION__, __VA_ARGS__)
#define LOG_INFO(...) SimpleBluez::log_info(__FILE__, __LINE__, __PRETTY_FUNCTION__, __VA_ARGS__)
#define LOG_DEBUG(...) SimpleBluez::log_debug(__FILE__, __LINE__, __PRETTY_FUNCTION__, __VA_ARGS__)
#define LOG_VERBOSE(...) SimpleBluez::log_verbose(__FILE__, __LINE__, __PRETTY_FUNCTION__, __VA_ARGS__)
