#include <simpleble_c/utils.h>

#include <simpleble_c/types.h>

simpleble_os_t simpleble_get_operating_system() {
#ifdef _WIN32
    return SIMPLEBLE_OS_WINDOWS;
#elif __APPLE__
    return SIMPLEBLE_OS_MACOS;
#elif __linux__
    return SIMPLEBLE_OS_LINUX;
#endif
}

const char* simpleble_get_version() { return SIMPLEBLE_VERSION; }
