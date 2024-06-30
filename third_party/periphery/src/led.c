/*
 * c-periphery
 * https://github.com/vsergeev/c-periphery
 * License: MIT
 */

#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "led.h"

#define P_PATH_MAX  256

struct led_handle {
    char name[64];
    unsigned int max_brightness;

    struct {
        int c_errno;
        char errmsg[96];
    } error;
};

static int _led_error(led_t *led, int code, int c_errno, const char *fmt, ...) {
    va_list ap;

    led->error.c_errno = c_errno;

    va_start(ap, fmt);
    vsnprintf(led->error.errmsg, sizeof(led->error.errmsg), fmt, ap);
    va_end(ap);

    /* Tack on strerror() and errno */
    if (c_errno) {
        char buf[64];
        strerror_r(c_errno, buf, sizeof(buf));
        snprintf(led->error.errmsg+strlen(led->error.errmsg), sizeof(led->error.errmsg)-strlen(led->error.errmsg), ": %s [errno %d]", buf, c_errno);
    }

    return code;
}

led_t *led_new(void) {
    led_t *led = calloc(1, sizeof(led_t));
    if (led == NULL)
        return NULL;

    return led;
}

int led_open(led_t *led, const char *name) {
    char led_path[P_PATH_MAX];
    int fd, ret;

    snprintf(led_path, sizeof(led_path), "/sys/class/leds/%s/brightness", name);

    if ((fd = open(led_path, O_RDWR)) < 0)
        return _led_error(led, LED_ERROR_OPEN, errno, "Opening LED: opening 'brightness'");

    close(fd);

    strncpy(led->name, name, sizeof(led->name) - 1);
    led->name[sizeof(led->name) - 1] = '\0';

    if ((ret = led_get_max_brightness(led, &led->max_brightness)) < 0)
        return ret;

    return 0;
}

int led_read(led_t *led, bool *value) {
    int ret;
    unsigned int brightness;

    if ((ret = led_get_brightness(led, &brightness)) < 0)
        return ret;

    *value = brightness != 0;

    return 0;
}

int led_write(led_t *led, bool value) {
    return led_set_brightness(led, value ? led->max_brightness : 0);
}

int led_close(led_t *led) {
    (void)led;
    return 0;
}

void led_free(led_t *led) {
    free(led);
}

int led_get_brightness(led_t *led, unsigned int *brightness) {
    char led_path[P_PATH_MAX];
    char buf[16];
    int fd, ret;

    snprintf(led_path, sizeof(led_path), "/sys/class/leds/%s/brightness", led->name);

    if ((fd = open(led_path, O_RDONLY)) < 0)
        return _led_error(led, LED_ERROR_IO, errno, "Opening LED 'brightness'");

    if ((ret = read(fd, buf, sizeof(buf))) < 0) {
        int errsv = errno;
        close(fd);
        return _led_error(led, LED_ERROR_IO, errsv, "Reading LED 'brightness'");
    }

    if (close(fd) < 0)
        return _led_error(led, LED_ERROR_IO, errno, "Closing LED 'brightness'");

    /* Null-terminate over newline */
    buf[ret] = '\0';

    *brightness = strtoul(buf, NULL, 10);

    return 0;
}

int led_get_max_brightness(led_t *led, unsigned int *max_brightness) {
    char led_path[P_PATH_MAX];
    char buf[16];
    int fd, ret;

    snprintf(led_path, sizeof(led_path), "/sys/class/leds/%s/max_brightness", led->name);

    if ((fd = open(led_path, O_RDONLY)) < 0)
        return _led_error(led, LED_ERROR_QUERY, errno, "Opening LED 'max_brightness'");

    if ((ret = read(fd, buf, sizeof(buf))) < 0) {
        int errsv = errno;
        close(fd);
        return _led_error(led, LED_ERROR_QUERY, errsv, "Reading LED 'max_brightness'");
    }

    if (close(fd) < 0)
        return _led_error(led, LED_ERROR_QUERY, errno, "Closing LED 'max_brightness'");

    /* Null-terminate over newline */
    buf[ret] = '\0';

    led->max_brightness = strtoul(buf, NULL, 10);

    *max_brightness = led->max_brightness;

    return 0;
}

int led_set_brightness(led_t *led, unsigned int brightness) {
    char led_path[P_PATH_MAX];
    char buf[16];
    int fd, len;

    if (brightness > led->max_brightness)
        return _led_error(led, LED_ERROR_ARG, 0, "Brightness out of bounds (max is %u)", led->max_brightness);

    snprintf(led_path, sizeof(led_path), "/sys/class/leds/%s/brightness", led->name);

    if ((fd = open(led_path, O_WRONLY)) < 0)
        return _led_error(led, LED_ERROR_IO, errno, "Opening LED 'brightness'");

    len = snprintf(buf, sizeof(buf), "%u\n", brightness);

    if (write(fd, buf, len) < 0) {
        int errsv = errno;
        close(fd);
        return _led_error(led, LED_ERROR_IO, errsv, "Writing LED 'brightness'");
    }

    if (close(fd) < 0)
        return _led_error(led, LED_ERROR_IO, errno, "Closing LED 'brightness'");

    return 0;
}

int led_name(led_t *led, char *str, size_t len) {
    if (!len)
        return 0;

    strncpy(str, led->name, len - 1);
    str[len - 1] = '\0';

    return 0;
}

int led_tostring(led_t *led, char *str, size_t len) {
    unsigned int brightness;
    char brightness_str[16];
    unsigned int max_brightness;
    char max_brightness_str[16];

    if (led_get_brightness(led, &brightness) < 0)
        strcpy(brightness_str, "<error>");
    else
        snprintf(brightness_str, sizeof(brightness_str), "%u", brightness);

    if (led_get_max_brightness(led, &max_brightness) < 0)
        strcpy(max_brightness_str, "<error>");
    else
        snprintf(max_brightness_str, sizeof(max_brightness_str), "%u", max_brightness);

    return snprintf(str, len, "LED %s (brightness=%s, max_brightness=%s)", led->name, brightness_str, max_brightness_str);
}

int led_errno(led_t *led) {
    return led->error.c_errno;
}

const char *led_errmsg(led_t *led) {
    return led->error.errmsg;
}
