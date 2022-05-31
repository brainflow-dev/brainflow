#include <simpleble/Utils.h>

namespace SimpleBLE {

OperatingSystem get_operating_system() {
#ifdef _WIN32
    return OperatingSystem::WINDOWS;
#elif __APPLE__
    return OperatingSystem::MACOS;
#elif __linux__
    return OperatingSystem::LINUX;
#endif
}

}  // namespace SimpleBLE