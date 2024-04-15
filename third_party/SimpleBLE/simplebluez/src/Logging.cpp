#include "Logging.h"
#include <simpledbus/external/logfwd.hpp>

#ifndef SIMPLEBLUEZ_LOG_LEVEL
#warning "SIMPLEBLUEZ_LOG_LEVEL not defined, using default value: FATAL"
#define SIMPLEBLUEZ_LOG_LEVEL FATAL
#endif

static const logfwd::level log_level = logfwd::level::SIMPLEBLUEZ_LOG_LEVEL;

namespace SimpleBluez {

void log_fatal(const std::string& file, uint32_t line, const std::string& function, const std::string& message) {
    if (log_level >= logfwd::level::FATAL) {
        logfwd::receive(logfwd::level::FATAL, "SimpleBluez", file, line, function, message);
    }
}

void log_error(const std::string& file, uint32_t line, const std::string& function, const std::string& message) {
    if (log_level >= logfwd::level::ERROR) {
        logfwd::receive(logfwd::level::ERROR, "SimpleBluez", file, line, function, message);
    }
}

void log_warn(const std::string& file, uint32_t line, const std::string& function, const std::string& message) {
    if (log_level >= logfwd::level::WARN) {
        logfwd::receive(logfwd::level::WARN, "SimpleBluez", file, line, function, message);
    }
}

void log_info(const std::string& file, uint32_t line, const std::string& function, const std::string& message) {
    if (log_level >= logfwd::level::INFO) {
        logfwd::receive(logfwd::level::INFO, "SimpleBluez", file, line, function, message);
    }
}

void log_debug(const std::string& file, uint32_t line, const std::string& function, const std::string& message) {
    if (log_level >= logfwd::level::DEBUG) {
        logfwd::receive(logfwd::level::DEBUG, "SimpleBluez", file, line, function, message);
    }
}

void log_verbose(const std::string& file, uint32_t line, const std::string& function, const std::string& message) {
    if (log_level >= logfwd::level::VERBOSE) {
        logfwd::receive(logfwd::level::VERBOSE, "SimpleBluez", file, line, function, message);
    }
}

}  // namespace SimpleBluez

__attribute__((weak)) void logfwd::receive(logfwd::level level, const std::string& module, const std::string& file,
                                           uint32_t line, const std::string& function, const std::string& message) {}
