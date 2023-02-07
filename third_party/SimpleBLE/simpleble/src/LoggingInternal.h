#pragma once

#include <fmt/core.h>
#include "simpleble/Logging.h"

#define SIMPLEBLE_LOG_LEVEL_NONE 0
#define SIMPLEBLE_LOG_LEVEL_FATAL 1
#define SIMPLEBLE_LOG_LEVEL_ERROR 2
#define SIMPLEBLE_LOG_LEVEL_WARN 3
#define SIMPLEBLE_LOG_LEVEL_INFO 4
#define SIMPLEBLE_LOG_LEVEL_DEBUG 5
#define SIMPLEBLE_LOG_LEVEL_VERBOSE 6

#ifndef SIMPLEBLE_LOG_LEVEL
#define SIMPLEBLE_LOG_LEVEL SIMPLEBLE_LOG_LEVEL_VERBOSE
#endif

// clang-format off

#if SIMPLEBLE_LOG_LEVEL == SIMPLEBLE_LOG_LEVEL_VERBOSE

#define SIMPLEBLE_LOG_FATAL(msg) SimpleBLE::Logging::Logger::get()->log(SimpleBLE::Logging::Level::Fatal, "SimpleBLE", __FILE__, __LINE__, __func__, msg)
#define SIMPLEBLE_LOG_ERROR(msg) SimpleBLE::Logging::Logger::get()->log(SimpleBLE::Logging::Level::Error, "SimpleBLE", __FILE__, __LINE__, __func__, msg)
#define SIMPLEBLE_LOG_WARN(msg) SimpleBLE::Logging::Logger::get()->log(SimpleBLE::Logging::Level::Warn, "SimpleBLE", __FILE__, __LINE__, __func__, msg)
#define SIMPLEBLE_LOG_INFO(msg) SimpleBLE::Logging::Logger::get()->log(SimpleBLE::Logging::Level::Info, "SimpleBLE", __FILE__, __LINE__, __func__, msg)
#define SIMPLEBLE_LOG_DEBUG(msg) SimpleBLE::Logging::Logger::get()->log(SimpleBLE::Logging::Level::Debug, "SimpleBLE", __FILE__, __LINE__, __func__, msg)
#define SIMPLEBLE_LOG_VERBOSE(msg) SimpleBLE::Logging::Logger::get()->log(SimpleBLE::Logging::Level::VERBOSE, "SimpleBLE", __FILE__, __LINE__, __func__, msg)

#elif SIMPLEBLE_LOG_LEVEL == SIMPLEBLE_LOG_LEVEL_DEBUG

#define SIMPLEBLE_LOG_FATAL(msg) SimpleBLE::Logging::Logger::get()->log(SimpleBLE::Logging::Level::Fatal, "SimpleBLE", __FILE__, __LINE__, __func__, msg)
#define SIMPLEBLE_LOG_ERROR(msg) SimpleBLE::Logging::Logger::get()->log(SimpleBLE::Logging::Level::Error, "SimpleBLE", __FILE__, __LINE__, __func__, msg)
#define SIMPLEBLE_LOG_WARN(msg) SimpleBLE::Logging::Logger::get()->log(SimpleBLE::Logging::Level::Warn, "SimpleBLE", __FILE__, __LINE__, __func__, msg)
#define SIMPLEBLE_LOG_INFO(msg) SimpleBLE::Logging::Logger::get()->log(SimpleBLE::Logging::Level::Info, "SimpleBLE", __FILE__, __LINE__, __func__, msg)
#define SIMPLEBLE_LOG_DEBUG(msg) SimpleBLE::Logging::Logger::get()->log(SimpleBLE::Logging::Level::Debug, "SimpleBLE", __FILE__, __LINE__, __func__, msg)
#define SIMPLEBLE_LOG_VERBOSE(msg)

#elif SIMPLEBLE_LOG_LEVEL == SIMPLEBLE_LOG_LEVEL_INFO

#define SIMPLEBLE_LOG_FATAL(msg) SimpleBLE::Logging::Logger::get()->log(SimpleBLE::Logging::Level::Fatal, "SimpleBLE", __FILE__, __LINE__, __func__, msg)
#define SIMPLEBLE_LOG_ERROR(msg) SimpleBLE::Logging::Logger::get()->log(SimpleBLE::Logging::Level::Error, "SimpleBLE", __FILE__, __LINE__, __func__, msg)
#define SIMPLEBLE_LOG_WARN(msg) SimpleBLE::Logging::Logger::get()->log(SimpleBLE::Logging::Level::Warn, "SimpleBLE", __FILE__, __LINE__, __func__, msg)
#define SIMPLEBLE_LOG_INFO(msg) SimpleBLE::Logging::Logger::get()->log(SimpleBLE::Logging::Level::Info, "SimpleBLE", __FILE__, __LINE__, __func__, msg)
#define SIMPLEBLE_LOG_DEBUG(msg) 
#define SIMPLEBLE_LOG_VERBOSE(msg)

#elif SIMPLEBLE_LOG_LEVEL == SIMPLEBLE_LOG_LEVEL_WARN

#define SIMPLEBLE_LOG_FATAL(msg) SimpleBLE::Logging::Logger::get()->log(SimpleBLE::Logging::Level::Fatal, "SimpleBLE", __FILE__, __LINE__, __func__, msg)
#define SIMPLEBLE_LOG_ERROR(msg) SimpleBLE::Logging::Logger::get()->log(SimpleBLE::Logging::Level::Error, "SimpleBLE", __FILE__, __LINE__, __func__, msg)
#define SIMPLEBLE_LOG_WARN(msg) SimpleBLE::Logging::Logger::get()->log(SimpleBLE::Logging::Level::Warn, "SimpleBLE", __FILE__, __LINE__, __func__, msg)
#define SIMPLEBLE_LOG_INFO(msg) 
#define SIMPLEBLE_LOG_DEBUG(msg) 
#define SIMPLEBLE_LOG_VERBOSE(msg)

#elif SIMPLEBLE_LOG_LEVEL == SIMPLEBLE_LOG_LEVEL_ERROR

#define SIMPLEBLE_LOG_FATAL(msg) SimpleBLE::Logging::Logger::get()->log(SimpleBLE::Logging::Level::Fatal, "SimpleBLE", __FILE__, __LINE__, __func__, msg)
#define SIMPLEBLE_LOG_ERROR(msg) SimpleBLE::Logging::Logger::get()->log(SimpleBLE::Logging::Level::Error, "SimpleBLE", __FILE__, __LINE__, __func__, msg)
#define SIMPLEBLE_LOG_WARN(msg)
#define SIMPLEBLE_LOG_INFO(msg) 
#define SIMPLEBLE_LOG_DEBUG(msg) 
#define SIMPLEBLE_LOG_VERBOSE(msg)

#elif SIMPLEBLE_LOG_LEVEL == SIMPLEBLE_LOG_LEVEL_FATAL

#define SIMPLEBLE_LOG_FATAL(msg) SimpleBLE::Logging::Logger::get()->log(SimpleBLE::Logging::Level::Fatal, "SimpleBLE", __FILE__, __LINE__, __func__, msg)
#define SIMPLEBLE_LOG_ERROR(msg)
#define SIMPLEBLE_LOG_WARN(msg)
#define SIMPLEBLE_LOG_INFO(msg) 
#define SIMPLEBLE_LOG_DEBUG(msg) 
#define SIMPLEBLE_LOG_VERBOSE(msg)

#else

#define SIMPLEBLE_LOG_FATAL(msg)
#define SIMPLEBLE_LOG_ERROR(msg)
#define SIMPLEBLE_LOG_WARN(msg)
#define SIMPLEBLE_LOG_INFO(msg) 
#define SIMPLEBLE_LOG_DEBUG(msg) 
#define SIMPLEBLE_LOG_VERBOSE(msg)

#endif

// clang-format on
