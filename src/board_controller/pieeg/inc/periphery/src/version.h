/*
 * c-periphery
 * https://github.com/vsergeev/c-periphery
 * License: MIT
 */

#ifndef _PERIPHERY_VERSION_H
#define _PERIPHERY_VERSION_H

#ifdef __cplusplus
extern "C" {
#endif

#define PERIPHERY_VERSION_MAJOR 2
#define PERIPHERY_VERSION_MINOR 4
#define PERIPHERY_VERSION_PATCH 2

#ifndef PERIPHERY_VERSION_COMMIT
#define PERIPHERY_VERSION_COMMIT "unknown"
#endif

typedef struct periphery_version {
    unsigned int major;
    unsigned int minor;
    unsigned int patch;
    const char *commit_id;
} periphery_version_t;

const char *periphery_version(void);

const periphery_version_t *periphery_version_info(void);

#ifdef __cplusplus
}
#endif

#endif

