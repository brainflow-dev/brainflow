#include "simpleble/Advanced.h"

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

#include "simplejni/VM.hpp"

namespace SimpleBLE::Advanced::Android {

JavaVM* get_jvm() { return SimpleJNI::VM::jvm(); }
void set_jvm(JavaVM* jvm) { SimpleJNI::VM::jvm(jvm); }

}  // namespace SimpleBLE::Advanced::Android

#endif

#if defined(__linux__) && !defined(__ANDROID__)
namespace SimpleBLE::Advanced::Linux {}

#endif