#include <simpleble/Logging.h>
#include <simpleble_c/logging.h>

void simpleble_logging_set_level(simpleble_log_level_t level) {
    SimpleBLE::Logging::Logger::get()->set_level(static_cast<SimpleBLE::Logging::Level>(level));
}

void simpleble_logging_set_callback(simpleble_log_callback_t callback) {
    SimpleBLE::Logging::Logger::get()->set_callback(
        [callback](SimpleBLE::Logging::Level level, const std::string& module, const std::string& file, uint32_t line,
                   const std::string& function, const std::string& message) {
            try {
                callback(static_cast<simpleble_log_level_t>(level), module.c_str(), file.c_str(), line,
                         function.c_str(), message.c_str());
            } catch (...) {
                // Clearly, if the logging callback throws an exception, we should not crash.
            }
        });
}
