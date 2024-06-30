/*
 * c-periphery
 * https://github.com/vsergeev/c-periphery
 * License: MIT
 */

#ifndef _PERIPHERY_GPIO_INTERNAL_H
#define _PERIPHERY_GPIO_INTERNAL_H

#include <stdarg.h>

#include "gpio.h"

/*********************************************************************************/
/* Operations table and handle structure */
/*********************************************************************************/

struct gpio_ops {
    int (*read)(gpio_t *gpio, bool *value);
    int (*write)(gpio_t *gpio, bool value);
    int (*read_event)(gpio_t *gpio, gpio_edge_t *edge, uint64_t *timestamp);
    int (*poll)(gpio_t *gpio, int timeout_ms);
    int (*close)(gpio_t *gpio);
    int (*get_direction)(gpio_t *gpio, gpio_direction_t *direction);
    int (*get_edge)(gpio_t *gpio, gpio_edge_t *edge);
    int (*get_bias)(gpio_t *gpio, gpio_bias_t *bias);
    int (*get_drive)(gpio_t *gpio, gpio_drive_t *drive);
    int (*get_inverted)(gpio_t *gpio, bool *inverted);
    int (*set_direction)(gpio_t *gpio, gpio_direction_t direction);
    int (*set_edge)(gpio_t *gpio, gpio_edge_t edge);
    int (*set_bias)(gpio_t *gpio, gpio_bias_t bias);
    int (*set_drive)(gpio_t *gpio, gpio_drive_t drive);
    int (*set_inverted)(gpio_t *gpio, bool inverted);
    unsigned int (*line)(gpio_t *gpio);
    int (*fd)(gpio_t *gpio);
    int (*name)(gpio_t *gpio, char *str, size_t len);
    int (*label)(gpio_t *gpio, char *str, size_t len);
    int (*chip_fd)(gpio_t *gpio);
    int (*chip_name)(gpio_t *gpio, char *str, size_t len);
    int (*chip_label)(gpio_t *gpio, char *str, size_t len);
    int (*tostring)(gpio_t *gpio, char *str, size_t len);
};

struct gpio_handle {
    const struct gpio_ops *ops;

    union {
        struct {
            unsigned int line;
            int line_fd;
            int chip_fd;
            gpio_direction_t direction;
            gpio_edge_t edge;
            gpio_bias_t bias;
            gpio_drive_t drive;
            bool inverted;
            char label[32];
        } cdev;
        struct {
            unsigned int line;
            int line_fd;
            bool exported;
        } sysfs;
    } u;

    /* error state */
    struct {
        int c_errno;
        char errmsg[96];
    } error;
};

/*********************************************************************************/
/* Common error formatting function */
/*********************************************************************************/

inline static int _gpio_error(gpio_t *gpio, int code, int c_errno, const char *fmt, ...) {
    va_list ap;

    gpio->error.c_errno = c_errno;

    va_start(ap, fmt);
    vsnprintf(gpio->error.errmsg, sizeof(gpio->error.errmsg), fmt, ap);
    va_end(ap);

    /* Tack on strerror() and errno */
    if (c_errno) {
        char buf[64];
        strerror_r(c_errno, buf, sizeof(buf));
        snprintf(gpio->error.errmsg+strlen(gpio->error.errmsg), sizeof(gpio->error.errmsg)-strlen(gpio->error.errmsg), ": %s [errno %d]", buf, c_errno);
    }

    return code;
}

#endif

