/*
 * c-periphery
 * https://github.com/vsergeev/c-periphery
 * License: MIT
 */

#ifndef _PERIPHERY_SERIAL_H
#define _PERIPHERY_SERIAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

enum serial_error_code {
    SERIAL_ERROR_ARG            = -1, /* Invalid arguments */
    SERIAL_ERROR_OPEN           = -2, /* Opening serial port */
    SERIAL_ERROR_QUERY          = -3, /* Querying serial port attributes */
    SERIAL_ERROR_CONFIGURE      = -4, /* Configuring serial port attributes */
    SERIAL_ERROR_IO             = -5, /* Reading/writing serial port */
    SERIAL_ERROR_CLOSE          = -6, /* Closing serial port */
};

typedef enum serial_parity {
    PARITY_NONE,
    PARITY_ODD,
    PARITY_EVEN,
} serial_parity_t;

typedef struct serial_handle serial_t;

/* Primary Functions */
serial_t *serial_new(void);
int serial_open(serial_t *serial, const char *path, uint32_t baudrate);
int serial_open_advanced(serial_t *serial, const char *path,
                         uint32_t baudrate, unsigned int databits,
                         serial_parity_t parity, unsigned int stopbits,
                         bool xonxoff, bool rtscts);
int serial_read(serial_t *serial, uint8_t *buf, size_t len, int timeout_ms);
int serial_write(serial_t *serial, const uint8_t *buf, size_t len);
int serial_flush(serial_t *serial);
int serial_input_waiting(serial_t *serial, unsigned int *count);
int serial_output_waiting(serial_t *serial, unsigned int *count);
int serial_poll(serial_t *serial, int timeout_ms);
int serial_close(serial_t *serial);
void serial_free(serial_t *serial);

/* Getters */
int serial_get_baudrate(serial_t *serial, uint32_t *baudrate);
int serial_get_databits(serial_t *serial, unsigned int *databits);
int serial_get_parity(serial_t *serial, serial_parity_t *parity);
int serial_get_stopbits(serial_t *serial, unsigned int *stopbits);
int serial_get_xonxoff(serial_t *serial, bool *xonxoff);
int serial_get_rtscts(serial_t *serial, bool *rtscts);
int serial_get_vmin(serial_t *serial, unsigned int *vmin);
int serial_get_vtime(serial_t *serial, float* vtime);

/* Setters */
int serial_set_baudrate(serial_t *serial, uint32_t baudrate);
int serial_set_databits(serial_t *serial, unsigned int databits);
int serial_set_parity(serial_t *serial, enum serial_parity parity);
int serial_set_stopbits(serial_t *serial, unsigned int stopbits);
int serial_set_xonxoff(serial_t *serial, bool enabled);
int serial_set_rtscts(serial_t *serial, bool enabled);
int serial_set_vmin(serial_t *serial, unsigned int vmin);
int serial_set_vtime(serial_t *serial, float vtime);

/* Miscellaneous */
int serial_fd(serial_t *serial);
int serial_tostring(serial_t *serial, char *str, size_t len);

/* Error Handling */
int serial_errno(serial_t *serial);
const char *serial_errmsg(serial_t *serial);

#ifdef __cplusplus
}
#endif

#endif

