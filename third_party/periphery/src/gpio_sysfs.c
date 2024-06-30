/*
 * c-periphery
 * https://github.com/vsergeev/c-periphery
 * License: MIT
 */

#define _XOPEN_SOURCE   600 /* for usleep() */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/stat.h>
#include <errno.h>

#include "gpio.h"
#include "gpio_internal.h"

/*********************************************************************************/
/* sysfs implementation */
/*********************************************************************************/

#define P_PATH_MAX  256

/* Delay between checks for successful GPIO export (100ms) */
#define GPIO_SYSFS_OPEN_DELAY      100000
/* Number of retries to check for successful GPIO exports */
#define GPIO_SYSFS_OPEN_RETRIES    10

static int gpio_sysfs_close(gpio_t *gpio) {
    char buf[16];
    int len, fd;

    if (gpio->u.sysfs.line_fd < 0)
        return 0;

    /* Close fd */
    if (close(gpio->u.sysfs.line_fd) < 0)
        return _gpio_error(gpio, GPIO_ERROR_CLOSE, errno, "Closing GPIO 'value'");

    gpio->u.sysfs.line_fd = -1;

    /* Unexport the GPIO, if we exported it */
    if (gpio->u.sysfs.exported) {
        len = snprintf(buf, sizeof(buf), "%u\n", gpio->u.sysfs.line);

        if ((fd = open("/sys/class/gpio/unexport", O_WRONLY)) < 0)
            return _gpio_error(gpio, GPIO_ERROR_CLOSE, errno, "Closing GPIO: opening 'unexport'");

        if (write(fd, buf, len) < 0) {
            int errsv = errno;
            close(fd);
            return _gpio_error(gpio, GPIO_ERROR_CLOSE, errsv, "Closing GPIO: writing 'unexport'");
        }

        if (close(fd) < 0)
            return _gpio_error(gpio, GPIO_ERROR_CLOSE, errno, "Closing GPIO: closing 'unexport'");

        gpio->u.sysfs.exported = false;
    }

    return 0;
}

static int gpio_sysfs_read(gpio_t *gpio, bool *value) {
    char buf[2];

    /* Read fd */
    if (read(gpio->u.sysfs.line_fd, buf, 2) < 0)
        return _gpio_error(gpio, GPIO_ERROR_IO, errno, "Reading GPIO 'value'");

    /* Rewind */
    if (lseek(gpio->u.sysfs.line_fd, 0, SEEK_SET) < 0)
        return _gpio_error(gpio, GPIO_ERROR_IO, errno, "Rewinding GPIO 'value'");

    if (buf[0] == '0')
        *value = false;
    else if (buf[0] == '1')
        *value = true;
    else
        return _gpio_error(gpio, GPIO_ERROR_IO, 0, "Unknown GPIO value");

    return 0;
}

static int gpio_sysfs_write(gpio_t *gpio, bool value) {
    static const char *value_str[2] = {"0\n", "1\n"};

    /* Write fd */
    if (write(gpio->u.sysfs.line_fd, value_str[value], 2) < 0)
        return _gpio_error(gpio, GPIO_ERROR_IO, errno, "Writing GPIO 'value'");

    /* Rewind */
    if (lseek(gpio->u.sysfs.line_fd, 0, SEEK_SET) < 0)
        return _gpio_error(gpio, GPIO_ERROR_IO, errno, "Rewinding GPIO 'value'");

    return 0;
}

static int gpio_sysfs_read_event(gpio_t *gpio, gpio_edge_t *edge, uint64_t *timestamp) {
    (void)edge;
    (void)timestamp;
    return _gpio_error(gpio, GPIO_ERROR_UNSUPPORTED, 0, "GPIO of type sysfs does not support read event");
}

static int gpio_sysfs_poll(gpio_t *gpio, int timeout_ms) {
    struct pollfd fds[1];
    int ret;

    /* Poll */
    fds[0].fd = gpio->u.sysfs.line_fd;
    fds[0].events = POLLPRI | POLLERR;
    if ((ret = poll(fds, 1, timeout_ms)) < 0)
        return _gpio_error(gpio, GPIO_ERROR_IO, errno, "Polling GPIO 'value'");

    /* GPIO edge interrupt occurred */
    if (ret) {
        /* Rewind */
        if (lseek(gpio->u.sysfs.line_fd, 0, SEEK_SET) < 0)
            return _gpio_error(gpio, GPIO_ERROR_IO, errno, "Rewinding GPIO 'value'");

        return 1;
    }

    /* Timed out */
    return 0;
}

static int gpio_sysfs_set_direction(gpio_t *gpio, gpio_direction_t direction) {
    char gpio_path[P_PATH_MAX];
    const char *buf;
    int fd;

    if (direction == GPIO_DIR_IN)
        buf = "in\n";
    else if (direction == GPIO_DIR_OUT)
        buf = "out\n";
    else if (direction == GPIO_DIR_OUT_LOW)
        buf = "low\n";
    else if (direction == GPIO_DIR_OUT_HIGH)
        buf = "high\n";
    else
        return _gpio_error(gpio, GPIO_ERROR_ARG, 0, "Invalid GPIO direction (can be in, out, low, high)");

    /* Write direction */
    snprintf(gpio_path, sizeof(gpio_path), "/sys/class/gpio/gpio%u/direction", gpio->u.sysfs.line);

    if ((fd = open(gpio_path, O_WRONLY)) < 0)
        return _gpio_error(gpio, GPIO_ERROR_CONFIGURE, errno, "Opening GPIO 'direction'");

    if (write(fd, buf, strlen(buf)) < 0) {
        int errsv = errno;
        close(fd);
        return _gpio_error(gpio, GPIO_ERROR_CONFIGURE, errsv, "Writing GPIO 'direction'");
    }

    if (close(fd) < 0)
        return _gpio_error(gpio, GPIO_ERROR_CONFIGURE, errno, "Closing GPIO 'direction'");

    return 0;
}

static int gpio_sysfs_get_direction(gpio_t *gpio, gpio_direction_t *direction) {
    char gpio_path[P_PATH_MAX];
    char buf[8];
    int fd, ret;

    /* Read direction */
    snprintf(gpio_path, sizeof(gpio_path), "/sys/class/gpio/gpio%u/direction", gpio->u.sysfs.line);

    if ((fd = open(gpio_path, O_RDONLY)) < 0)
        return _gpio_error(gpio, GPIO_ERROR_QUERY, errno, "Opening GPIO 'direction'");

    if ((ret = read(fd, buf, sizeof(buf))) < 0) {
        int errsv = errno;
        close(fd);
        return _gpio_error(gpio, GPIO_ERROR_QUERY, errsv, "Reading GPIO 'direction'");
    }

    if (close(fd) < 0)
        return _gpio_error(gpio, GPIO_ERROR_QUERY, errno, "Closing GPIO 'direction'");

    buf[ret] = '\0';

    if (strcmp(buf, "in\n") == 0)
        *direction = GPIO_DIR_IN;
    else if (strcmp(buf, "out\n") == 0)
        *direction = GPIO_DIR_OUT;
    else
        return _gpio_error(gpio, GPIO_ERROR_QUERY, 0, "Unknown GPIO direction");

    return 0;
}

static int gpio_sysfs_set_edge(gpio_t *gpio, gpio_edge_t edge) {
    char gpio_path[P_PATH_MAX];
    const char *buf;
    int fd;

    if (edge == GPIO_EDGE_NONE)
        buf = "none\n";
    else if (edge == GPIO_EDGE_RISING)
        buf = "rising\n";
    else if (edge == GPIO_EDGE_FALLING)
        buf = "falling\n";
    else if (edge == GPIO_EDGE_BOTH)
        buf = "both\n";
    else
        return _gpio_error(gpio, GPIO_ERROR_ARG, 0, "Invalid GPIO interrupt edge (can be none, rising, falling, both)");

    /* Write edge */
    snprintf(gpio_path, sizeof(gpio_path), "/sys/class/gpio/gpio%u/edge", gpio->u.sysfs.line);

    if ((fd = open(gpio_path, O_WRONLY)) < 0)
        return _gpio_error(gpio, GPIO_ERROR_CONFIGURE, errno, "Opening GPIO 'edge'");

    if (write(fd, buf, strlen(buf)) < 0) {
        int errsv = errno;
        close(fd);
        return _gpio_error(gpio, GPIO_ERROR_CONFIGURE, errsv, "Writing GPIO 'edge'");
    }

    if (close(fd) < 0)
        return _gpio_error(gpio, GPIO_ERROR_CONFIGURE, errno, "Closing GPIO 'edge'");

    return 0;
}

static int gpio_sysfs_get_edge(gpio_t *gpio, gpio_edge_t *edge) {
    char gpio_path[P_PATH_MAX];
    char buf[16];
    int fd, ret;

    /* Read edge */
    snprintf(gpio_path, sizeof(gpio_path), "/sys/class/gpio/gpio%u/edge", gpio->u.sysfs.line);

    if ((fd = open(gpio_path, O_RDONLY)) < 0)
        return _gpio_error(gpio, GPIO_ERROR_QUERY, errno, "Opening GPIO 'edge'");

    if ((ret = read(fd, buf, sizeof(buf))) < 0) {
        int errsv = errno;
        close(fd);
        return _gpio_error(gpio, GPIO_ERROR_QUERY, errsv, "Reading GPIO 'edge'");
    }

    if (close(fd) < 0)
        return _gpio_error(gpio, GPIO_ERROR_QUERY, errno, "Closing GPIO 'edge'");

    buf[ret] = '\0';

    if (strcmp(buf, "none\n") == 0)
        *edge = GPIO_EDGE_NONE;
    else if (strcmp(buf, "rising\n") == 0)
        *edge = GPIO_EDGE_RISING;
    else if (strcmp(buf, "falling\n") == 0)
        *edge = GPIO_EDGE_FALLING;
    else if (strcmp(buf, "both\n") == 0)
        *edge = GPIO_EDGE_BOTH;
    else
        return _gpio_error(gpio, GPIO_ERROR_QUERY, 0, "Unknown GPIO edge");

    return 0;
}

static int gpio_sysfs_set_bias(gpio_t *gpio, gpio_bias_t bias) {
    (void)bias;
    return _gpio_error(gpio, GPIO_ERROR_UNSUPPORTED, 0, "GPIO of type sysfs does not support line bias attribute");
}

static int gpio_sysfs_get_bias(gpio_t *gpio, gpio_bias_t *bias) {
    (void)bias;
    return _gpio_error(gpio, GPIO_ERROR_UNSUPPORTED, 0, "GPIO of type sysfs does not support line bias attribute");
}

static int gpio_sysfs_set_drive(gpio_t *gpio, gpio_drive_t drive) {
    (void)drive;
    return _gpio_error(gpio, GPIO_ERROR_UNSUPPORTED, 0, "GPIO of type sysfs does not support line drive attribute");
}

static int gpio_sysfs_get_drive(gpio_t *gpio, gpio_drive_t *drive) {
    (void)drive;
    return _gpio_error(gpio, GPIO_ERROR_UNSUPPORTED, 0, "GPIO of type sysfs does not support line drive attribute");
}

static int gpio_sysfs_set_inverted(gpio_t *gpio, bool inverted) {
    char gpio_path[P_PATH_MAX];
    static const char *inverted_str[2] = {"0\n", "1\n"};
    int fd;

    /* Write active_low */
    snprintf(gpio_path, sizeof(gpio_path), "/sys/class/gpio/gpio%u/active_low", gpio->u.sysfs.line);

    if ((fd = open(gpio_path, O_WRONLY)) < 0)
        return _gpio_error(gpio, GPIO_ERROR_CONFIGURE, errno, "Opening GPIO 'active_low'");

    if (write(fd, inverted_str[inverted], strlen(inverted_str[inverted])) < 0) {
        int errsv = errno;
        close(fd);
        return _gpio_error(gpio, GPIO_ERROR_CONFIGURE, errsv, "Writing GPIO 'active_low'");
    }

    if (close(fd) < 0)
        return _gpio_error(gpio, GPIO_ERROR_CONFIGURE, errno, "Closing GPIO 'active_low'");

    return 0;
}

static int gpio_sysfs_get_inverted(gpio_t *gpio, bool *inverted) {
    char gpio_path[P_PATH_MAX];
    char buf[4];
    int fd, ret;

    /* Read active_low */
    snprintf(gpio_path, sizeof(gpio_path), "/sys/class/gpio/gpio%u/active_low", gpio->u.sysfs.line);

    if ((fd = open(gpio_path, O_RDONLY)) < 0)
        return _gpio_error(gpio, GPIO_ERROR_QUERY, errno, "Opening GPIO 'active_low'");

    if ((ret = read(fd, buf, sizeof(buf))) < 0) {
        int errsv = errno;
        close(fd);
        return _gpio_error(gpio, GPIO_ERROR_QUERY, errsv, "Reading GPIO 'active_low'");
    }

    if (close(fd) < 0)
        return _gpio_error(gpio, GPIO_ERROR_QUERY, errno, "Closing GPIO 'active_low'");

    buf[ret] = '\0';

    if (buf[0] == '0')
        *inverted = false;
    else if (buf[0] == '1')
        *inverted = true;
    else
        return _gpio_error(gpio, GPIO_ERROR_QUERY, 0, "Unknown GPIO active_low value");

    return 0;
}

static unsigned int gpio_sysfs_line(gpio_t *gpio) {
    return gpio->u.sysfs.line;
}

static int gpio_sysfs_fd(gpio_t *gpio) {
    return gpio->u.sysfs.line_fd;
}

static int gpio_sysfs_name(gpio_t *gpio, char *str, size_t len) {
    (void)gpio;
    if (len)
        str[0] = '\0';

    return 0;
}

static int gpio_sysfs_label(gpio_t *gpio, char *str, size_t len) {
    (void)gpio;
    if (len)
        str[0] = '\0';

    return 0;
}

static int gpio_sysfs_chip_fd(gpio_t *gpio) {
    return _gpio_error(gpio, GPIO_ERROR_UNSUPPORTED, 0, "GPIO of type sysfs has no chip fd");
}

static int gpio_sysfs_chip_name(gpio_t *gpio, char *str, size_t len) {
    int ret;
    char gpio_path[P_PATH_MAX];
    char gpiochip_path[P_PATH_MAX];

    if (!len)
        return 0;

    /* Form path to device */
    snprintf(gpio_path, sizeof(gpio_path), "/sys/class/gpio/gpio%u/device", gpio->u.sysfs.line);

    /* Resolve symlink to gpiochip */
    if ((ret = readlink(gpio_path, gpiochip_path, sizeof(gpiochip_path))) < 0)
        return _gpio_error(gpio, GPIO_ERROR_QUERY, errno, "Reading GPIO chip symlink");

    /* Null-terminate symlink path */
    gpiochip_path[(ret < P_PATH_MAX) ? ret : (P_PATH_MAX - 1)] = '\0';

    /* Find last / in symlink path */
    const char *sep = strrchr(gpiochip_path, '/');
    if (!sep)
        return _gpio_error(gpio, GPIO_ERROR_QUERY, 0, "Invalid GPIO chip symlink");

    strncpy(str, sep + 1, len - 1);
    str[len - 1] = '\0';

    return 0;
}

static int gpio_sysfs_chip_label(gpio_t *gpio, char *str, size_t len) {
    char gpio_path[P_PATH_MAX];
    char chip_name[32];
    int fd, ret;

    if (!len)
        return 0;

    if ((ret = gpio_sysfs_chip_name(gpio, chip_name, sizeof(chip_name))) < 0)
        return ret;

    /* Read gpiochip label */
    snprintf(gpio_path, sizeof(gpio_path), "/sys/class/gpio/%s/label", chip_name);

    if ((fd = open(gpio_path, O_RDONLY)) < 0)
        return _gpio_error(gpio, GPIO_ERROR_QUERY, errno, "Opening GPIO chip 'label'");

    if ((ret = read(fd, str, len)) < 0) {
        int errsv = errno;
        close(fd);
        return _gpio_error(gpio, GPIO_ERROR_QUERY, errsv, "Reading GPIO chip 'label'");
    }

    if (close(fd) < 0)
        return _gpio_error(gpio, GPIO_ERROR_QUERY, errno, "Closing GPIO 'label'");

    str[ret - 1] = '\0';

    return 0;
}

static int gpio_sysfs_tostring(gpio_t *gpio, char *str, size_t len) {
    gpio_direction_t direction;
    const char *direction_str;
    gpio_edge_t edge;
    const char *edge_str;
    bool inverted;
    const char *inverted_str;
    char chip_name[32];
    const char *chip_name_str;
    char chip_label[32];
    const char *chip_label_str;

    if (gpio_sysfs_get_direction(gpio, &direction) < 0)
        direction_str = "<error>";
    else
        direction_str = (direction == GPIO_DIR_IN) ? "in" :
                        (direction == GPIO_DIR_OUT) ? "out" : "unknown";

    if (gpio_sysfs_get_edge(gpio, &edge) < 0)
        edge_str = "<error>";
    else
        edge_str = (edge == GPIO_EDGE_NONE) ? "none" :
                   (edge == GPIO_EDGE_RISING) ? "rising" :
                   (edge == GPIO_EDGE_FALLING) ? "falling" :
                   (edge == GPIO_EDGE_BOTH) ? "both" : "unknown";

    if (gpio_sysfs_get_inverted(gpio, &inverted) < 0)
        inverted_str = "<error>";
    else
        inverted_str = inverted ? "true" : "false";

    if (gpio_sysfs_chip_name(gpio, chip_name, sizeof(chip_name)) < 0)
        chip_name_str = "<error>";
    else
        chip_name_str = chip_name;

    if (gpio_sysfs_chip_label(gpio, chip_label, sizeof(chip_label)) < 0)
        chip_label_str = "<error>";
    else
        chip_label_str = chip_label;

    return snprintf(str, len, "GPIO %u (fd=%d, direction=%s, edge=%s, inverted=%s, chip_name=\"%s\", chip_label=\"%s\", type=sysfs)",
                    gpio->u.sysfs.line, gpio->u.sysfs.line_fd, direction_str, edge_str, inverted_str, chip_name_str, chip_label_str);
}

const struct gpio_ops gpio_sysfs_ops = {
    .read = gpio_sysfs_read,
    .write = gpio_sysfs_write,
    .read_event = gpio_sysfs_read_event,
    .poll = gpio_sysfs_poll,
    .close = gpio_sysfs_close,
    .get_direction = gpio_sysfs_get_direction,
    .get_edge = gpio_sysfs_get_edge,
    .get_bias = gpio_sysfs_get_bias,
    .get_drive = gpio_sysfs_get_drive,
    .get_inverted = gpio_sysfs_get_inverted,
    .set_direction = gpio_sysfs_set_direction,
    .set_edge = gpio_sysfs_set_edge,
    .set_bias = gpio_sysfs_set_bias,
    .set_drive = gpio_sysfs_set_drive,
    .set_inverted = gpio_sysfs_set_inverted,
    .line = gpio_sysfs_line,
    .fd = gpio_sysfs_fd,
    .name = gpio_sysfs_name,
    .label = gpio_sysfs_label,
    .chip_fd = gpio_sysfs_chip_fd,
    .chip_name = gpio_sysfs_chip_name,
    .chip_label = gpio_sysfs_chip_label,
    .tostring = gpio_sysfs_tostring,
};

int gpio_open_sysfs(gpio_t *gpio, unsigned int line, gpio_direction_t direction) {
    char gpio_path[P_PATH_MAX];
    struct stat stat_buf;
    char buf[16];
    int len, fd, ret;
    bool exported = false;

    if (direction != GPIO_DIR_IN && direction != GPIO_DIR_OUT && direction != GPIO_DIR_OUT_LOW && direction != GPIO_DIR_OUT_HIGH)
        return _gpio_error(gpio, GPIO_ERROR_ARG, 0, "Invalid GPIO direction (can be in, out, low, high)");

    /* Check if GPIO directory exists */
    snprintf(gpio_path, sizeof(gpio_path), "/sys/class/gpio/gpio%u", line);
    if (stat(gpio_path, &stat_buf) < 0) {
        /* Write line number to export file */
        len = snprintf(buf, sizeof(buf), "%u\n", line);

        if ((fd = open("/sys/class/gpio/export", O_WRONLY)) < 0)
            return _gpio_error(gpio, GPIO_ERROR_OPEN, errno, "Opening GPIO: opening 'export'");

        if (write(fd, buf, len) < 0) {
            int errsv = errno;
            close(fd);
            return _gpio_error(gpio, GPIO_ERROR_OPEN, errsv, "Opening GPIO: writing 'export'");
        }

        if (close(fd) < 0)
            return _gpio_error(gpio, GPIO_ERROR_OPEN, errno, "Opening GPIO: closing 'export'");

        /* Wait until GPIO directory appears */
        unsigned int retry_count;
        for (retry_count = 0; retry_count < GPIO_SYSFS_OPEN_RETRIES; retry_count++) {
            int ret = stat(gpio_path, &stat_buf);
            if (ret == 0) {
                exported = true;
                break;
            } else if (ret < 0 && errno != ENOENT) {
                return _gpio_error(gpio, GPIO_ERROR_OPEN, errno, "Opening GPIO: stat 'gpio%u/' after export", line);
            }

            usleep(GPIO_SYSFS_OPEN_DELAY);
        }

        if (retry_count == GPIO_SYSFS_OPEN_RETRIES)
            return _gpio_error(gpio, GPIO_ERROR_OPEN, 0, "Opening GPIO: waiting for 'gpio%u/' timed out", line);

        /* Write direction, looping in case of EACCES errors due to delayed
         * udev permission rule application after export */
        const char *dir = (direction == GPIO_DIR_OUT) ? "out\n" :
                          (direction == GPIO_DIR_OUT_HIGH) ? "high\n" :
                          (direction == GPIO_DIR_OUT_LOW) ? "low\n" : "in\n";

        snprintf(gpio_path, sizeof(gpio_path), "/sys/class/gpio/gpio%u/direction", line);

        for (retry_count = 0; retry_count < GPIO_SYSFS_OPEN_RETRIES; retry_count++) {
            if ((fd = open(gpio_path, O_WRONLY)) >= 0) {
                if (write(fd, dir, strlen(dir)) < 0) {
                    int errsv = errno;
                    close(fd);
                    return _gpio_error(gpio, GPIO_ERROR_CONFIGURE, errsv, "Writing GPIO 'direction'");
                }

                if (close(fd) < 0)
                    return _gpio_error(gpio, GPIO_ERROR_CONFIGURE, errno, "Closing GPIO 'direction'");

                break;
            } else if (errno != EACCES) {
                return _gpio_error(gpio, GPIO_ERROR_OPEN, errno, "Opening GPIO: opening 'gpio%u/direction'", line);
            }

            usleep(GPIO_SYSFS_OPEN_DELAY);
        }

        if (retry_count == GPIO_SYSFS_OPEN_RETRIES)
            return _gpio_error(gpio, GPIO_ERROR_OPEN, errno, "Opening GPIO: opening 'gpio%u/direction'", line);
    }

    /* Open value */
    snprintf(gpio_path, sizeof(gpio_path), "/sys/class/gpio/gpio%u/value", line);
    if ((fd = open(gpio_path, O_RDWR)) < 0)
        return _gpio_error(gpio, GPIO_ERROR_OPEN, errno, "Opening GPIO 'gpio%u/value'", line);

    memset(gpio, 0, sizeof(gpio_t));
    gpio->ops = &gpio_sysfs_ops;
    gpio->u.sysfs.line = line;
    gpio->u.sysfs.line_fd = fd;
    gpio->u.sysfs.exported = exported;

    if (!exported) {
        ret = gpio_sysfs_set_direction(gpio, direction);
        if (ret < 0)
            return ret;
    }

    ret = gpio_sysfs_set_inverted(gpio, false);
    if (ret < 0)
        return ret;

    return 0;
}

