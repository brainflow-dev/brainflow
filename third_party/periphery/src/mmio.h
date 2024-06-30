/*
 * c-periphery
 * https://github.com/vsergeev/c-periphery
 * License: MIT
 */

#ifndef _PERIPHERY_MMIO_H
#define _PERIPHERY_MMIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

enum mmio_error_code {
    MMIO_ERROR_ARG          = -1, /* Invalid arguments */
    MMIO_ERROR_OPEN         = -2, /* Opening MMIO */
    MMIO_ERROR_CLOSE        = -3, /* Closing MMIO */
};

typedef struct mmio_handle mmio_t;

/* Primary Functions */
mmio_t *mmio_new(void);
int mmio_open(mmio_t *mmio, uintptr_t base, size_t size);
int mmio_open_advanced(mmio_t *mmio, uintptr_t base, size_t size, const char *path);
void *mmio_ptr(mmio_t *mmio);
int mmio_read32(mmio_t *mmio, uintptr_t offset, uint32_t *value);
int mmio_read16(mmio_t *mmio, uintptr_t offset, uint16_t *value);
int mmio_read8(mmio_t *mmio, uintptr_t offset, uint8_t *value);
int mmio_read(mmio_t *mmio, uintptr_t offset, uint8_t *buf, size_t len);
int mmio_write32(mmio_t *mmio, uintptr_t offset, uint32_t value);
int mmio_write16(mmio_t *mmio, uintptr_t offset, uint16_t value);
int mmio_write8(mmio_t *mmio, uintptr_t offset, uint8_t value);
int mmio_write(mmio_t *mmio, uintptr_t offset, const uint8_t *buf, size_t len);
int mmio_close(mmio_t *mmio);
void mmio_free(mmio_t *mmio);

/* Miscellaneous */
uintptr_t mmio_base(mmio_t *mmio);
size_t mmio_size(mmio_t *mmio);
int mmio_tostring(mmio_t *mmio, char *str, size_t len);

/* Error Handling */
int mmio_errno(mmio_t *mmio);
const char *mmio_errmsg(mmio_t *mmio);

#ifdef __cplusplus
}
#endif

#endif

