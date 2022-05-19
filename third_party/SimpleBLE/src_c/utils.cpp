#include <simpleble_c/utils.h>

#include <simpleble_c/types.h>

simpleble_os_t get_operating_system() {
#ifdef _WIN32
    return SIMPLEBLE_OS_WINDOWS;
#elif __APPLE__
    return SIMPLEBLE_OS_MACOS;
#elif __linux__
    return SIMPLEBLE_OS_LINUX;
#endif
}
