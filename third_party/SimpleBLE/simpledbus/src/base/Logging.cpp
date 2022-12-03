#include "Logging.h"
#include <simpledbus/external/logfwd.hpp>

#ifndef SIMPLEDBUS_LOG_LEVEL
#warning "SIMPLEDBUS_LOG_LEVEL not defined, using default value: ERROR"
#define SIMPLEDBUS_LOG_LEVEL ERROR
#endif

static const logfwd::level log_level = logfwd::level::SIMPLEDBUS_LOG_LEVEL;

namespace SimpleDBus {

void log_fatal(const std::string& file, uint32_t line, const std::string& function, const std::string& message) {
    if (log_level >= logfwd::level::FATAL) {
        logfwd::receive(logfwd::level::FATAL, "SimpleDBus", file, line, function, message);
    }
}

void log_error(const std::string& file, uint32_t line, const std::string& function, const std::string& message) {
    if (log_level >= logfwd::level::ERROR) {
        logfwd::receive(logfwd::level::ERROR, "SimpleDBus", file, line, function, message);
    }
}

void log_warn(const std::string& file, uint32_t line, const std::string& function, const std::string& message) {
    if (log_level >= logfwd::level::WARN) {
        logfwd::receive(logfwd::level::WARN, "SimpleDBus", file, line, function, message);
    }
}

void log_info(const std::string& file, uint32_t line, const std::string& function, const std::string& message) {
    if (log_level >= logfwd::level::INFO) {
        logfwd::receive(logfwd::level::INFO, "SimpleDBus", file, line, function, message);
    }
}

void log_debug(const std::string& file, uint32_t line, const std::string& function, const std::string& message) {
    if (log_level >= logfwd::level::DEBUG) {
        logfwd::receive(logfwd::level::DEBUG, "SimpleDBus", file, line, function, message);
    }
}

void log_verbose(const std::string& file, uint32_t line, const std::string& function, const std::string& message) {
    if (log_level >= logfwd::level::VERBOSE) {
        logfwd::receive(logfwd::level::VERBOSE, "SimpleDBus", file, line, function, message);
    }
}

}  // namespace SimpleDBus

__attribute__((weak)) void logfwd::receive(logfwd::level level, const std::string& module, const std::string& file,
                                           uint32_t line, const std::string& function, const std::string& message) {}
