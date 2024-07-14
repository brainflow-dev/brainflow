/*
 * c-periphery
 * https://github.com/vsergeev/c-periphery
 * License: MIT
 */

#define _XOPEN_SOURCE   600 /* for POLLRDNORM */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/stat.h>
#include <errno.h>

#include <sys/ioctl.h>

#include "gpio.h"
#include "gpio_internal.h"

extern const struct gpio_ops gpio_cdev_ops;
extern const struct gpio_ops gpio_sysfs_ops;

gpio_t *gpio_new(void) {
    gpio_t *gpio = calloc(1, sizeof(gpio_t));
    if (gpio == NULL)
        return NULL;

#if PERIPHERY_GPIO_CDEV_SUPPORT
    gpio->ops = &gpio_cdev_ops;
    gpio->u.cdev.line_fd = -1;
    gpio->u.cdev.chip_fd = -1;
#else
    gpio->ops = &gpio_sysfs_ops;
    gpio->u.sysfs.line_fd = -1;
#endif

    return gpio;
}

int gpio_read(gpio_t *gpio, bool *value) {
    return gpio->ops->read(gpio, value);
}

int gpio_write(gpio_t *gpio, bool value) {
    return gpio->ops->write(gpio, value);
}

int gpio_poll(gpio_t *gpio, int timeout_ms) {
    return gpio->ops->poll(gpio, timeout_ms);
}

int gpio_close(gpio_t *gpio) {
    return gpio->ops->close(gpio);
}

void gpio_free(gpio_t *gpio) {
    free(gpio);
}

int gpio_read_event(gpio_t *gpio, gpio_edge_t *edge, uint64_t *timestamp) {
    return gpio->ops->read_event(gpio, edge, timestamp);
}

int gpio_poll_multiple(gpio_t **gpios, size_t count, int timeout_ms, bool *gpios_ready) {
    struct pollfd fds[count];
    int ret;

    /* Setup pollfd structs */
    for (size_t i = 0; i < count; i++) {
        fds[i].fd = gpio_fd(gpios[i]);
        fds[i].events = (gpios[i]->ops == &gpio_sysfs_ops) ?
                            (POLLPRI | POLLERR) : (POLLIN | POLLRDNORM);
        if (gpios_ready)
            gpios_ready[i] = false;
    }

    /* Poll */
    if ((ret = poll(fds, count, timeout_ms)) < 0)
        return GPIO_ERROR_IO;

    /* Event occurred */
    if (ret) {
        for (size_t i = 0; i < count; i++) {
            /* Set ready GPIOs */
            if (gpios_ready)
                gpios_ready[i] = fds[i].revents != 0;

            /* Rewind GPIO if it is a sysfs GPIO */
            if (gpios[i]->ops == &gpio_sysfs_ops) {
                if (lseek(gpios[i]->u.sysfs.line_fd, 0, SEEK_SET) < 0)
                    return GPIO_ERROR_IO;
            }
        }

        return ret;
    }

    /* Timed out */
    return 0;
}

int gpio_get_direction(gpio_t *gpio, gpio_direction_t *direction) {
    return gpio->ops->get_direction(gpio, direction);
}

int gpio_get_edge(gpio_t *gpio, gpio_edge_t *edge) {
    return gpio->ops->get_edge(gpio, edge);
}

int gpio_get_bias(gpio_t *gpio, gpio_bias_t *bias) {
    return gpio->ops->get_bias(gpio, bias);
}

int gpio_get_drive(gpio_t *gpio, gpio_drive_t *drive) {
    return gpio->ops->get_drive(gpio, drive);
}

int gpio_get_inverted(gpio_t *gpio, bool *inverted) {
    return gpio->ops->get_inverted(gpio, inverted);
}

int gpio_set_direction(gpio_t *gpio, gpio_direction_t direction) {
    return gpio->ops->set_direction(gpio, direction);
}

int gpio_set_edge(gpio_t *gpio, gpio_edge_t edge) {
    return gpio->ops->set_edge(gpio, edge);
}

int gpio_set_bias(gpio_t *gpio, gpio_bias_t bias) {
    return gpio->ops->set_bias(gpio, bias);
}

int gpio_set_drive(gpio_t *gpio, gpio_drive_t drive) {
    return gpio->ops->set_drive(gpio, drive);
}

int gpio_set_inverted(gpio_t *gpio, bool inverted) {
    return gpio->ops->set_inverted(gpio, inverted);
}

unsigned int gpio_line(gpio_t *gpio) {
    return gpio->ops->line(gpio);
}

int gpio_fd(gpio_t *gpio) {
    return gpio->ops->fd(gpio);
}

int gpio_name(gpio_t *gpio, char *str, size_t len) {
    return gpio->ops->name(gpio, str, len);
}

int gpio_label(gpio_t *gpio, char *str, size_t len) {
    return gpio->ops->label(gpio, str, len);
}

int gpio_chip_fd(gpio_t *gpio) {
    return gpio->ops->chip_fd(gpio);
}

int gpio_chip_name(gpio_t *gpio, char *str, size_t len) {
    return gpio->ops->chip_name(gpio, str, len);
}

int gpio_chip_label(gpio_t *gpio, char *str, size_t len) {
    return gpio->ops->chip_label(gpio, str, len);
}

int gpio_tostring(gpio_t *gpio, char *str, size_t len) {
    return gpio->ops->tostring(gpio, str, len);
}

int gpio_errno(gpio_t *gpio) {
    return gpio->error.c_errno;
}

const char *gpio_errmsg(gpio_t *gpio) {
    return gpio->error.errmsg;
}

#if !PERIPHERY_GPIO_CDEV_SUPPORT

int gpio_open(gpio_t *gpio, const char *path, unsigned int line, gpio_direction_t direction)  {
    (void)path;
    (void)line;
    (void)direction;
    return _gpio_error(gpio, GPIO_ERROR_UNSUPPORTED, 0, "c-periphery library built without character device GPIO support.");
}

int gpio_open_name(gpio_t *gpio, const char *path, const char *name, gpio_direction_t direction) {
    (void)gpio;
    (void)path;
    (void)name;
    (void)direction;
    return _gpio_error(gpio, GPIO_ERROR_UNSUPPORTED, 0, "c-periphery library built without character device GPIO support.");
}

int gpio_open_advanced(gpio_t *gpio, const char *path, unsigned int line, const gpio_config_t *config) {
    (void)path;
    (void)line;
    (void)config;
    return _gpio_error(gpio, GPIO_ERROR_UNSUPPORTED, 0, "c-periphery library built without character device GPIO support.");
}

int gpio_open_name_advanced(gpio_t *gpio, const char *path, const char *name, const gpio_config_t *config) {
    (void)path;
    (void)name;
    (void)config;
    return _gpio_error(gpio, GPIO_ERROR_UNSUPPORTED, 0, "c-periphery library built without character device GPIO support.");
}

#endif

