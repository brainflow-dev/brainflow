#pragma once

#include <simpleble/export.h>

#if __APPLE__
#include "TargetConditionals.h"
#endif

/**
 * Advanced Features
 *
 * The functions presented in this namespace are OS-specific backdoors that are
 * not part of the standard SimpleBLE API, which allow the user to access
 * low-level details of the implementation for advanced use cases.
 *
 * These functions should be used with caution.
 */

#if defined(_WIN32)
namespace SimpleBLE::Advanced::Windows {}

#endif

#if TARGET_OS_OSX
namespace SimpleBLE::Advanced::MacOS {}

#endif

#if TARGET_OS_IOS
namespace SimpleBLE::Advanced::iOS {}

#endif

#if defined(__ANDROID__)

#include <jni.h>

namespace SimpleBLE::Advanced::Android {

JavaVM* SIMPLEBLE_EXPORT get_jvm();
void SIMPLEBLE_EXPORT set_jvm(JavaVM* jvm);

}  // namespace SimpleBLE::Advanced::Android

#endif

#if defined(__linux__) && !defined(__ANDROID__)
namespace SimpleBLE::Advanced::Linux {}

#endif