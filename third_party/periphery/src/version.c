/*
 * c-periphery
 * https://github.com/vsergeev/c-periphery
 * License: MIT
 */

#include "version.h"

const char *periphery_version(void) {
    #define _STRINGIFY(s) #s
    #define STRINGIFY(s) _STRINGIFY(s)
    return "v" STRINGIFY(PERIPHERY_VERSION_MAJOR) "." STRINGIFY(PERIPHERY_VERSION_MINOR) "." STRINGIFY(PERIPHERY_VERSION_PATCH);
}

const periphery_version_t *periphery_version_info(void) {
    static const periphery_version_t version = {
        .major = PERIPHERY_VERSION_MAJOR,
        .minor = PERIPHERY_VERSION_MINOR,
        .patch = PERIPHERY_VERSION_PATCH,
        .commit_id = PERIPHERY_VERSION_COMMIT,
    };
    return &version;
}
