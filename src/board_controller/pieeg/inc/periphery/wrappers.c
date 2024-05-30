#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>

// Wrapper for strerror_r
char* __xpg_strerror_r(int errnum, char* buf, size_t buflen) {
    strerror_r(errnum, buf, buflen);
    return buf;
}

// Wrapper for __errno_location
int* __errno_location(void) {
    return &errno;
}

// Wrapper for __xstat
int __xstat(int ver, const char * path, struct stat * stat_buf) {
    return stat(path, stat_buf);
}
