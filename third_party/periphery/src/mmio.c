/*
 * c-periphery
 * https://github.com/vsergeev/c-periphery
 * License: MIT
 */

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include "mmio.h"

struct mmio_handle {
    uintptr_t base, aligned_base;
    size_t size, aligned_size;
    void *ptr;

    struct {
        int c_errno;
        char errmsg[96];
    } error;
};

static int _mmio_error(mmio_t *mmio, int code, int c_errno, const char *fmt, ...) {
    va_list ap;

    mmio->error.c_errno = c_errno;

    va_start(ap, fmt);
    vsnprintf(mmio->error.errmsg, sizeof(mmio->error.errmsg), fmt, ap);
    va_end(ap);

    /* Tack on strerror() and errno */
    if (c_errno) {
        char buf[64];
        strerror_r(c_errno, buf, sizeof(buf));
        snprintf(mmio->error.errmsg+strlen(mmio->error.errmsg), sizeof(mmio->error.errmsg)-strlen(mmio->error.errmsg), ": %s [errno %d]", buf, c_errno);
    }

    return code;
}

mmio_t *mmio_new(void) {
    return calloc(1, sizeof(mmio_t));
}

void mmio_free(mmio_t *mmio) {
    free(mmio);
}

int mmio_open(mmio_t *mmio, uintptr_t base, size_t size) {
    return mmio_open_advanced(mmio, base, size, "/dev/mem");
}

int mmio_open_advanced(mmio_t *mmio, uintptr_t base, size_t size, const char *path) {
    int fd;

    memset(mmio, 0, sizeof(mmio_t));
    mmio->base = base;
    mmio->size = size;
    mmio->aligned_base = mmio->base - (mmio->base % sysconf(_SC_PAGESIZE));
    mmio->aligned_size = mmio->size + (mmio->base - mmio->aligned_base);

    /* Open memory */
    if ((fd = open(path, O_RDWR | O_SYNC)) < 0)
        return _mmio_error(mmio, MMIO_ERROR_OPEN, errno, "Opening %s", path);

    /* Map memory */
    if ((mmio->ptr = mmap(0, mmio->aligned_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, mmio->aligned_base)) == MAP_FAILED) {
        int errsv = errno;
        close(fd);
        return _mmio_error(mmio, MMIO_ERROR_OPEN, errsv, "Mapping memory");
    }

    /* Close memory */
    if (close(fd) < 0) {
        int errsv = errno;
        munmap(mmio->ptr, mmio->aligned_size);
        mmio->ptr = 0;
        return _mmio_error(mmio, MMIO_ERROR_OPEN, errsv, "Closing %s", path);
    }

    return 0;
}

void *mmio_ptr(mmio_t *mmio) {
    return (void *)((uint8_t *)mmio->ptr + (mmio->base - mmio->aligned_base));
}

/* WARNING: These functions may trigger a bus fault on some CPUs if an
 * unaligned address is accessed! */

int mmio_read32(mmio_t *mmio, uintptr_t offset, uint32_t *value) {
    offset += (mmio->base - mmio->aligned_base);
    if ((offset+4) > mmio->aligned_size)
        return _mmio_error(mmio, MMIO_ERROR_ARG, 0, "Offset out of bounds");

    *value = *(volatile uint32_t *)(((volatile uint8_t *)mmio->ptr) + offset);
    return 0;
}

int mmio_read16(mmio_t *mmio, uintptr_t offset, uint16_t *value) {
    offset += (mmio->base - mmio->aligned_base);
    if ((offset+2) > mmio->aligned_size)
        return _mmio_error(mmio, MMIO_ERROR_ARG, 0, "Offset out of bounds");

    *value = *(volatile uint16_t *)(((volatile uint8_t *)mmio->ptr) + offset);
    return 0;
}

int mmio_read8(mmio_t *mmio, uintptr_t offset, uint8_t *value) {
    offset += (mmio->base - mmio->aligned_base);
    if ((offset+1) > mmio->aligned_size)
        return _mmio_error(mmio, MMIO_ERROR_ARG, 0, "Offset out of bounds");

    *value = *(volatile uint8_t *)(((volatile uint8_t *)mmio->ptr) + offset);
    return 0;
}

int mmio_read(mmio_t *mmio, uintptr_t offset, uint8_t *buf, size_t len) {
    offset += (mmio->base - mmio->aligned_base);
    if ((offset+len) > mmio->aligned_size)
        return _mmio_error(mmio, MMIO_ERROR_ARG, 0, "Offset out of bounds");

    memcpy((void *)buf, (const void *)(((volatile uint8_t *)mmio->ptr) + offset), len);
    return 0;
}

int mmio_write32(mmio_t *mmio, uintptr_t offset, uint32_t value) {
    offset += (mmio->base - mmio->aligned_base);
    if ((offset+4) > mmio->aligned_size)
        return _mmio_error(mmio, MMIO_ERROR_ARG, 0, "Offset out of bounds");

    *(volatile uint32_t *)(((volatile uint8_t *)mmio->ptr) + offset) = value;
    return 0;
}

int mmio_write16(mmio_t *mmio, uintptr_t offset, uint16_t value) {
    offset += (mmio->base - mmio->aligned_base);
    if ((offset+2) > mmio->aligned_size)
        return _mmio_error(mmio, MMIO_ERROR_ARG, 0, "Offset out of bounds");

    *(volatile uint16_t *)(((volatile uint8_t *)mmio->ptr) + offset) = value;
    return 0;
}

int mmio_write8(mmio_t *mmio, uintptr_t offset, uint8_t value) {
    offset += (mmio->base - mmio->aligned_base);
    if ((offset+1) > mmio->aligned_size)
        return _mmio_error(mmio, MMIO_ERROR_ARG, 0, "Offset out of bounds");

    *(volatile uint8_t *)(((volatile uint8_t *)mmio->ptr) + offset) = value;
    return 0;
}

int mmio_write(mmio_t *mmio, uintptr_t offset, const uint8_t *buf, size_t len) {
    offset += (mmio->base - mmio->aligned_base);
    if ((offset+len) > mmio->aligned_size)
        return _mmio_error(mmio, MMIO_ERROR_ARG, 0, "Offset out of bounds");

    memcpy((void *)(((volatile uint8_t *)mmio->ptr) + offset), (const void *)buf, len);
    return 0;
}

int mmio_close(mmio_t *mmio) {
    if (!mmio->ptr)
        return 0;

    /* Unmap memory */
    if (munmap(mmio->ptr, mmio->aligned_size) < 0)
        return _mmio_error(mmio, MMIO_ERROR_CLOSE, errno, "Unmapping memory");

    mmio->ptr = 0;

    return 0;
}

int mmio_tostring(mmio_t *mmio, char *str, size_t len) {
    return snprintf(str, len, "MMIO 0x%08zx (ptr=%p, size=%zu)", mmio->base, mmio->ptr, mmio->size);
}

const char *mmio_errmsg(mmio_t *mmio) {
    return mmio->error.errmsg;
}

int mmio_errno(mmio_t *mmio) {
    return mmio->error.c_errno;
}

uintptr_t mmio_base(mmio_t *mmio) {
    return mmio->base;
}

size_t mmio_size(mmio_t *mmio) {
    return mmio->size;
}

