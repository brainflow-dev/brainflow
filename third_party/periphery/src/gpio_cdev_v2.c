/*
 * c-periphery
 * https://github.com/vsergeev/c-periphery
 * License: MIT
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <errno.h>

#include "gpio.h"
#include "gpio_internal.h"

#if PERIPHERY_GPIO_CDEV_SUPPORT == 2
#include <linux/gpio.h>
#endif

/*********************************************************************************/
/* cdev v2 implementation */
/*********************************************************************************/

#if PERIPHERY_GPIO_CDEV_SUPPORT == 2

static int _gpio_cdev_reopen(gpio_t *gpio, gpio_direction_t direction, gpio_edge_t edge, gpio_bias_t bias, gpio_drive_t drive, bool inverted) {
    uint32_t flags = 0;

    if (bias == GPIO_BIAS_PULL_UP)
        flags |= GPIO_V2_LINE_FLAG_BIAS_PULL_UP;
    else if (bias == GPIO_BIAS_PULL_DOWN)
        flags |= GPIO_V2_LINE_FLAG_BIAS_PULL_DOWN;
    else if (bias == GPIO_BIAS_DISABLE)
        flags |= GPIO_V2_LINE_FLAG_BIAS_DISABLED;

    if (drive == GPIO_DRIVE_OPEN_DRAIN)
        flags |= GPIO_V2_LINE_FLAG_OPEN_DRAIN;
    else if (drive == GPIO_DRIVE_OPEN_SOURCE)
        flags |= GPIO_V2_LINE_FLAG_OPEN_SOURCE;

    if (inverted)
        flags |= GPIO_V2_LINE_FLAG_ACTIVE_LOW;

    /* FIXME this should really use GPIO_V2_LINE_SET_CONFIG_IOCTL instead of
     * closing and reopening, especially to preserve output value on
     * configuration changes */

    if (gpio->u.cdev.line_fd >= 0) {
        if (close(gpio->u.cdev.line_fd) < 0)
            return _gpio_error(gpio, GPIO_ERROR_CLOSE, errno, "Closing GPIO line");

        gpio->u.cdev.line_fd = -1;
    }

    if (direction == GPIO_DIR_IN) {
        struct gpio_v2_line_request line_request = {0};

        flags |= GPIO_V2_LINE_FLAG_INPUT;
        flags |= (edge == GPIO_EDGE_RISING) ? GPIO_V2_LINE_FLAG_EDGE_RISING :
                 (edge == GPIO_EDGE_FALLING) ? GPIO_V2_LINE_FLAG_EDGE_FALLING :
                 (edge == GPIO_EDGE_BOTH) ? (GPIO_V2_LINE_FLAG_EDGE_RISING | GPIO_V2_LINE_FLAG_EDGE_FALLING) : 0;
        flags |= (edge != GPIO_EDGE_NONE) ? GPIO_V2_LINE_FLAG_EVENT_CLOCK_REALTIME : 0;

        line_request.offsets[0] = gpio->u.cdev.line;
        strncpy(line_request.consumer, gpio->u.cdev.label, sizeof(line_request.consumer) - 1);
        line_request.consumer[sizeof(line_request.consumer) - 1] = '\0';
        line_request.config.flags = flags;
        line_request.num_lines = 1;

        if (ioctl(gpio->u.cdev.chip_fd, GPIO_V2_GET_LINE_IOCTL, &line_request) < 0)
            return _gpio_error(gpio, GPIO_ERROR_OPEN, errno, "Opening input line handle");

        gpio->u.cdev.line_fd = line_request.fd;
    } else {
        struct gpio_v2_line_request line_request = {0};

        bool initial_value = (direction == GPIO_DIR_OUT_HIGH) ? true : false;
        initial_value ^= inverted;

        flags |= GPIO_V2_LINE_FLAG_OUTPUT;

        line_request.offsets[0] = gpio->u.cdev.line;
        strncpy(line_request.consumer, gpio->u.cdev.label, sizeof(line_request.consumer) - 1);
        line_request.consumer[sizeof(line_request.consumer) - 1] = '\0';
        line_request.config.flags = flags;
        line_request.config.num_attrs = 1;
        line_request.config.attrs[0].attr.id = GPIO_V2_LINE_ATTR_ID_OUTPUT_VALUES;
        line_request.config.attrs[0].attr.values = initial_value & 0x1;
        line_request.config.attrs[0].mask = 1;
        line_request.num_lines = 1;

        if (ioctl(gpio->u.cdev.chip_fd, GPIO_V2_GET_LINE_IOCTL, &line_request) < 0)
            return _gpio_error(gpio, GPIO_ERROR_OPEN, errno, "Opening output line handle");

        gpio->u.cdev.line_fd = line_request.fd;
    }

    gpio->u.cdev.direction = (direction == GPIO_DIR_IN) ? GPIO_DIR_IN : GPIO_DIR_OUT;
    gpio->u.cdev.edge = edge;
    gpio->u.cdev.bias = bias;
    gpio->u.cdev.drive = drive;
    gpio->u.cdev.inverted = inverted;

    return 0;
}

static int gpio_cdev_read(gpio_t *gpio, bool *value) {
    struct gpio_v2_line_values line_values = {0, 1};

    if (ioctl(gpio->u.cdev.line_fd, GPIO_V2_LINE_GET_VALUES_IOCTL, &line_values) < 0)
        return _gpio_error(gpio, GPIO_ERROR_IO, errno, "Getting line value");

    *value = line_values.bits & 0x1;

    return 0;
}

static int gpio_cdev_write(gpio_t *gpio, bool value) {
    struct gpio_v2_line_values line_values = {0, 1};

    if (gpio->u.cdev.direction != GPIO_DIR_OUT)
        return _gpio_error(gpio, GPIO_ERROR_INVALID_OPERATION, 0, "Invalid operation: cannot write to input GPIO");

    line_values.bits = value & 0x1;

    if (ioctl(gpio->u.cdev.line_fd, GPIO_V2_LINE_SET_VALUES_IOCTL, &line_values) < 0)
        return _gpio_error(gpio, GPIO_ERROR_IO, errno, "Setting line value");

    return 0;
}

static int gpio_cdev_read_event(gpio_t *gpio, gpio_edge_t *edge, uint64_t *timestamp) {
    struct gpio_v2_line_event line_event = {0};

    if (gpio->u.cdev.direction != GPIO_DIR_IN)
        return _gpio_error(gpio, GPIO_ERROR_INVALID_OPERATION, 0, "Invalid operation: cannot read event of output GPIO");
    else if (gpio->u.cdev.edge == GPIO_EDGE_NONE)
        return _gpio_error(gpio, GPIO_ERROR_INVALID_OPERATION, 0, "Invalid operation: GPIO edge not set");

    if (read(gpio->u.cdev.line_fd, &line_event, sizeof(line_event)) < (ssize_t)sizeof(line_event))
        return _gpio_error(gpio, GPIO_ERROR_IO, errno, "Reading GPIO event");

    if (edge)
        *edge = (line_event.id == GPIO_V2_LINE_EVENT_RISING_EDGE) ? GPIO_EDGE_RISING :
                (line_event.id == GPIO_V2_LINE_EVENT_FALLING_EDGE) ? GPIO_EDGE_FALLING : GPIO_EDGE_NONE;
    if (timestamp)
        *timestamp = line_event.timestamp_ns;

    return 0;
}

static int gpio_cdev_poll(gpio_t *gpio, int timeout_ms) {
    struct pollfd fds[1];
    int ret;

    if (gpio->u.cdev.direction != GPIO_DIR_IN)
        return _gpio_error(gpio, GPIO_ERROR_INVALID_OPERATION, 0, "Invalid operation: cannot poll output GPIO");

    fds[0].fd = gpio->u.cdev.line_fd;
    fds[0].events = POLLIN | POLLPRI | POLLERR;
    if ((ret = poll(fds, 1, timeout_ms)) < 0)
        return _gpio_error(gpio, GPIO_ERROR_IO, errno, "Polling GPIO line");

    return ret > 0;
}

static int gpio_cdev_close(gpio_t *gpio) {
    /* Close line fd */
    if (gpio->u.cdev.line_fd >= 0) {
        if (close(gpio->u.cdev.line_fd) < 0)
            return _gpio_error(gpio, GPIO_ERROR_CLOSE, errno, "Closing GPIO line");

        gpio->u.cdev.line_fd = -1;
    }

    /* Close chip fd */
    if (gpio->u.cdev.chip_fd >= 0) {
        if (close(gpio->u.cdev.chip_fd) < 0)
            return _gpio_error(gpio, GPIO_ERROR_CLOSE, errno, "Closing GPIO chip");

        gpio->u.cdev.chip_fd = -1;
    }

    gpio->u.cdev.edge = GPIO_EDGE_NONE;
    gpio->u.cdev.direction = GPIO_DIR_IN;

    return 0;
}

static int gpio_cdev_get_direction(gpio_t *gpio, gpio_direction_t *direction) {
    *direction = gpio->u.cdev.direction;
    return 0;
}

static int gpio_cdev_get_edge(gpio_t *gpio, gpio_edge_t *edge) {
    *edge = gpio->u.cdev.edge;
    return 0;
}

static int gpio_cdev_get_bias(gpio_t *gpio, gpio_bias_t *bias) {
    *bias = gpio->u.cdev.bias;
    return 0;
}

static int gpio_cdev_get_drive(gpio_t *gpio, gpio_drive_t *drive) {
    *drive = gpio->u.cdev.drive;
    return 0;
}

static int gpio_cdev_get_inverted(gpio_t *gpio, bool *inverted) {
    *inverted = gpio->u.cdev.inverted;
    return 0;
}

static int gpio_cdev_set_direction(gpio_t *gpio, gpio_direction_t direction) {
    if (direction != GPIO_DIR_IN && direction != GPIO_DIR_OUT && direction != GPIO_DIR_OUT_LOW && direction != GPIO_DIR_OUT_HIGH)
        return _gpio_error(gpio, GPIO_ERROR_ARG, 0, "Invalid GPIO direction (can be in, out, low, high)");

    if (gpio->u.cdev.direction == direction)
        return 0;

    return _gpio_cdev_reopen(gpio, direction, GPIO_EDGE_NONE, gpio->u.cdev.bias, gpio->u.cdev.drive, gpio->u.cdev.inverted);
}

static int gpio_cdev_set_edge(gpio_t *gpio, gpio_edge_t edge) {
    if (edge != GPIO_EDGE_NONE && edge != GPIO_EDGE_RISING && edge != GPIO_EDGE_FALLING && edge != GPIO_EDGE_BOTH)
        return _gpio_error(gpio, GPIO_ERROR_ARG, 0, "Invalid GPIO interrupt edge (can be none, rising, falling, both)");

    if (gpio->u.cdev.direction != GPIO_DIR_IN)
        return _gpio_error(gpio, GPIO_ERROR_INVALID_OPERATION, 0, "Invalid operation: cannot set edge on output GPIO");

    if (gpio->u.cdev.edge == edge)
        return 0;

    return _gpio_cdev_reopen(gpio, gpio->u.cdev.direction, edge, gpio->u.cdev.bias, gpio->u.cdev.drive, gpio->u.cdev.inverted);
}

static int gpio_cdev_set_bias(gpio_t *gpio, gpio_bias_t bias) {
    if (bias != GPIO_BIAS_DEFAULT && bias != GPIO_BIAS_PULL_UP && bias != GPIO_BIAS_PULL_DOWN && bias != GPIO_BIAS_DISABLE)
        return _gpio_error(gpio, GPIO_ERROR_ARG, 0, "Invalid GPIO line bias (can be default, pull_up, pull_down, disable)");

    if (gpio->u.cdev.bias == bias)
        return 0;

    return _gpio_cdev_reopen(gpio, gpio->u.cdev.direction, gpio->u.cdev.edge, bias, gpio->u.cdev.drive, gpio->u.cdev.inverted);
}

static int gpio_cdev_set_drive(gpio_t *gpio, gpio_drive_t drive) {
    if (drive != GPIO_DRIVE_DEFAULT && drive != GPIO_DRIVE_OPEN_DRAIN && drive != GPIO_DRIVE_OPEN_SOURCE)
        return _gpio_error(gpio, GPIO_ERROR_ARG, 0, "Invalid GPIO line drive (can be default, open_drain, open_source)");

    if (gpio->u.cdev.direction != GPIO_DIR_OUT && drive != GPIO_DRIVE_DEFAULT)
        return _gpio_error(gpio, GPIO_ERROR_INVALID_OPERATION, 0, "Invalid operation: cannot set line drive on input GPIO");

    if (gpio->u.cdev.drive == drive)
        return 0;

    return _gpio_cdev_reopen(gpio, gpio->u.cdev.direction, gpio->u.cdev.edge, gpio->u.cdev.bias, drive, gpio->u.cdev.inverted);
}

static int gpio_cdev_set_inverted(gpio_t *gpio, bool inverted) {
    if (gpio->u.cdev.inverted == inverted)
        return 0;

    return _gpio_cdev_reopen(gpio, gpio->u.cdev.direction, gpio->u.cdev.edge, gpio->u.cdev.bias, gpio->u.cdev.drive, inverted);
}

static unsigned int gpio_cdev_line(gpio_t *gpio) {
    return gpio->u.cdev.line;
}

static int gpio_cdev_fd(gpio_t *gpio) {
    return gpio->u.cdev.line_fd;
}

static int gpio_cdev_name(gpio_t *gpio, char *str, size_t len) {
    struct gpio_v2_line_info line_info = {0};

    if (!len)
        return 0;

    line_info.offset = gpio->u.cdev.line;

    if (ioctl(gpio->u.cdev.chip_fd, GPIO_V2_GET_LINEINFO_IOCTL, &line_info) < 0)
        return _gpio_error(gpio, GPIO_ERROR_QUERY, errno, "Querying GPIO line info for line %u", gpio->u.cdev.line);

    strncpy(str, line_info.name, len - 1);
    str[len - 1] = '\0';

    return 0;
}

static int gpio_cdev_label(gpio_t *gpio, char *str, size_t len) {
    struct gpio_v2_line_info line_info = {0};

    if (!len)
        return 0;

    line_info.offset = gpio->u.cdev.line;

    if (ioctl(gpio->u.cdev.chip_fd, GPIO_V2_GET_LINEINFO_IOCTL, &line_info) < 0)
        return _gpio_error(gpio, GPIO_ERROR_QUERY, errno, "Querying GPIO line info for line %u", gpio->u.cdev.line);

    strncpy(str, line_info.consumer, len - 1);
    str[len - 1] = '\0';

    return 0;
}

static int gpio_cdev_chip_fd(gpio_t *gpio) {
    return gpio->u.cdev.chip_fd;
}

static int gpio_cdev_chip_name(gpio_t *gpio, char *str, size_t len) {
    struct gpiochip_info chip_info = {0};

    if (!len)
        return 0;

    if (ioctl(gpio->u.cdev.chip_fd, GPIO_GET_CHIPINFO_IOCTL, &chip_info) < 0)
        return _gpio_error(gpio, GPIO_ERROR_QUERY, errno, "Querying GPIO chip info");

    strncpy(str, chip_info.name, len - 1);
    str[len - 1] = '\0';

    return 0;
}

static int gpio_cdev_chip_label(gpio_t *gpio, char *str, size_t len) {
    struct gpiochip_info chip_info = {0};

    if (!len)
        return 0;

    if (ioctl(gpio->u.cdev.chip_fd, GPIO_GET_CHIPINFO_IOCTL, &chip_info) < 0)
        return _gpio_error(gpio, GPIO_ERROR_QUERY, errno, "Querying GPIO chip info");

    strncpy(str, chip_info.label, len - 1);
    str[len - 1] = '\0';

    return 0;
}

static int gpio_cdev_tostring(gpio_t *gpio, char *str, size_t len) {
    gpio_direction_t direction;
    const char *direction_str;
    gpio_edge_t edge;
    const char *edge_str;
    gpio_bias_t bias;
    const char *bias_str;
    gpio_drive_t drive;
    const char *drive_str;
    bool inverted;
    const char *inverted_str;
    char line_name[32];
    const char *line_name_str;
    char line_label[32];
    const char *line_label_str;
    char chip_name[32];
    const char *chip_name_str;
    char chip_label[32];
    const char *chip_label_str;

    if (gpio_cdev_get_direction(gpio, &direction) < 0)
        direction_str = "<error>";
    else
        direction_str = (direction == GPIO_DIR_IN) ? "in" :
                        (direction == GPIO_DIR_OUT) ? "out" : "unknown";

    if (gpio_cdev_get_edge(gpio, &edge) < 0)
        edge_str = "<error>";
    else
        edge_str = (edge == GPIO_EDGE_NONE) ? "none" :
                   (edge == GPIO_EDGE_RISING) ? "rising" :
                   (edge == GPIO_EDGE_FALLING) ? "falling" :
                   (edge == GPIO_EDGE_BOTH) ? "both" : "unknown";

    if (gpio_cdev_get_bias(gpio, &bias) < 0)
        bias_str = "<error>";
    else
        bias_str = (bias == GPIO_BIAS_DEFAULT) ? "default" :
                   (bias == GPIO_BIAS_PULL_UP) ? "pull_up" :
                   (bias == GPIO_BIAS_PULL_DOWN) ? "pull_down" :
                   (bias == GPIO_BIAS_DISABLE) ? "disable" : "unknown";

    if (gpio_cdev_get_drive(gpio, &drive) < 0)
        drive_str = "<error>";
    else
        drive_str = (drive == GPIO_DRIVE_DEFAULT) ? "default" :
                    (drive == GPIO_DRIVE_OPEN_DRAIN) ? "open_drain" :
                    (drive == GPIO_DRIVE_OPEN_SOURCE) ? "open_source" : "unknown";

    if (gpio_cdev_get_inverted(gpio, &inverted) < 0)
        inverted_str = "<error>";
    else
        inverted_str = inverted ? "true" : "false";

    if (gpio_cdev_name(gpio, line_name, sizeof(line_name)) < 0)
        line_name_str = "<error>";
    else
        line_name_str = line_name;

    if (gpio_cdev_label(gpio, line_label, sizeof(line_label)) < 0)
        line_label_str = "<error>";
    else
        line_label_str = line_label;

    if (gpio_cdev_chip_name(gpio, chip_name, sizeof(chip_name)) < 0)
        chip_name_str = "<error>";
    else
        chip_name_str = chip_name;

    if (gpio_cdev_chip_label(gpio, chip_label, sizeof(chip_label)) < 0)
        chip_label_str = "<error>";
    else
        chip_label_str = chip_label;

    return snprintf(str, len, "GPIO %u (name=\"%s\", label=\"%s\", line_fd=%d, chip_fd=%d, direction=%s, edge=%s, bias=%s, drive=%s, inverted=%s, chip_name=\"%s\", chip_label=\"%s\", type=cdev)",
                    gpio->u.cdev.line, line_name_str, line_label_str, gpio->u.cdev.line_fd, gpio->u.cdev.chip_fd, direction_str, edge_str, bias_str, drive_str, inverted_str, chip_name_str, chip_label_str);
}

const struct gpio_ops gpio_cdev_ops = {
    .read = gpio_cdev_read,
    .write = gpio_cdev_write,
    .read_event = gpio_cdev_read_event,
    .poll = gpio_cdev_poll,
    .close = gpio_cdev_close,
    .get_direction = gpio_cdev_get_direction,
    .get_edge = gpio_cdev_get_edge,
    .get_bias = gpio_cdev_get_bias,
    .get_drive = gpio_cdev_get_drive,
    .get_inverted = gpio_cdev_get_inverted,
    .set_direction = gpio_cdev_set_direction,
    .set_edge = gpio_cdev_set_edge,
    .set_bias = gpio_cdev_set_bias,
    .set_drive = gpio_cdev_set_drive,
    .set_inverted = gpio_cdev_set_inverted,
    .line = gpio_cdev_line,
    .fd = gpio_cdev_fd,
    .name = gpio_cdev_name,
    .label = gpio_cdev_label,
    .chip_fd = gpio_cdev_chip_fd,
    .chip_name = gpio_cdev_chip_name,
    .chip_label = gpio_cdev_chip_label,
    .tostring = gpio_cdev_tostring,
};

int gpio_open_advanced(gpio_t *gpio, const char *path, unsigned int line, const gpio_config_t *config) {
    int ret, fd;

    if (config->direction != GPIO_DIR_IN && config->direction != GPIO_DIR_OUT && config->direction != GPIO_DIR_OUT_LOW && config->direction != GPIO_DIR_OUT_HIGH)
        return _gpio_error(gpio, GPIO_ERROR_ARG, 0, "Invalid GPIO direction (can be in, out, low, high)");

    if (config->edge != GPIO_EDGE_NONE && config->edge != GPIO_EDGE_RISING && config->edge != GPIO_EDGE_FALLING && config->edge != GPIO_EDGE_BOTH)
        return _gpio_error(gpio, GPIO_ERROR_ARG, 0, "Invalid GPIO interrupt edge (can be none, rising, falling, both)");

    if (config->direction != GPIO_DIR_IN && config->edge != GPIO_EDGE_NONE)
        return _gpio_error(gpio, GPIO_ERROR_ARG, 0, "Invalid GPIO edge for output GPIO");

    if (config->bias != GPIO_BIAS_DEFAULT && config->bias != GPIO_BIAS_PULL_UP && config->bias != GPIO_BIAS_PULL_DOWN && config->bias != GPIO_BIAS_DISABLE)
        return _gpio_error(gpio, GPIO_ERROR_ARG, 0, "Invalid GPIO line bias (can be default, pull_up, pull_down, disable)");

    if (config->drive != GPIO_DRIVE_DEFAULT && config->drive != GPIO_DRIVE_OPEN_DRAIN && config->drive != GPIO_DRIVE_OPEN_SOURCE)
        return _gpio_error(gpio, GPIO_ERROR_ARG, 0, "Invalid GPIO line drive (can be default, open_drain, open_source)");

    if (config->direction == GPIO_DIR_IN && config->drive != GPIO_DRIVE_DEFAULT)
        return _gpio_error(gpio, GPIO_ERROR_ARG, 0, "Invalid GPIO line drive for input GPIO");

    /* Open GPIO chip */
    if ((fd = open(path, 0)) < 0)
        return _gpio_error(gpio, GPIO_ERROR_OPEN, errno, "Opening GPIO chip");

    memset(gpio, 0, sizeof(gpio_t));
    gpio->ops = &gpio_cdev_ops;
    gpio->u.cdev.line = line;
    gpio->u.cdev.line_fd = -1;
    gpio->u.cdev.chip_fd = fd;
    strncpy(gpio->u.cdev.label, config->label ? config->label : "periphery", sizeof(gpio->u.cdev.label) - 1);
    gpio->u.cdev.label[sizeof(gpio->u.cdev.label) - 1] = '\0';

    /* Open GPIO line */
    ret = _gpio_cdev_reopen(gpio, config->direction, config->edge, config->bias, config->drive, config->inverted);
    if (ret < 0) {
        close(gpio->u.cdev.chip_fd);
        gpio->u.cdev.chip_fd = -1;
        return ret;
    }

    return 0;
}

int gpio_open_name_advanced(gpio_t *gpio, const char *path, const char *name, const gpio_config_t *config) {
    int fd;

    /* Open GPIO chip */
    if ((fd = open(path, 0)) < 0)
        return _gpio_error(gpio, GPIO_ERROR_OPEN, errno, "Opening GPIO chip");

    /* Get chip info for number of lines */
    struct gpiochip_info chip_info = {0};
    if (ioctl(fd, GPIO_GET_CHIPINFO_IOCTL, &chip_info) < 0) {
        int errsv = errno;
        close(fd);
        return _gpio_error(gpio, GPIO_ERROR_QUERY, errsv, "Querying GPIO chip info");
    }

    /* Loop through every line */
    struct gpio_v2_line_info line_info = {0};
    unsigned int line;
    for (line = 0; line < chip_info.lines; line++) {
        line_info.offset = line;

        /* Get the line info */
        if (ioctl(fd, GPIO_V2_GET_LINEINFO_IOCTL, &line_info) < 0) {
            int errsv = errno;
            close(fd);
            return _gpio_error(gpio, GPIO_ERROR_QUERY, errsv, "Querying GPIO line info for line %u", line);
        }

        /* Compare the name */
        if (strcmp(line_info.name, name) == 0)
            break;
    }

    /* If no matching line name was found */
    if (line == chip_info.lines) {
        close(fd);
        return _gpio_error(gpio, GPIO_ERROR_NOT_FOUND, 0, "GPIO line \"%s\" not found by name", name);
    }

    if (close(fd) < 0)
        return _gpio_error(gpio, GPIO_ERROR_CLOSE, errno, "Closing GPIO chip");

    return gpio_open_advanced(gpio, path, line, config);
}

int gpio_open(gpio_t *gpio, const char *path, unsigned int line, gpio_direction_t direction) {
    gpio_config_t config = {
        .direction = direction,
        .edge = GPIO_EDGE_NONE,
        .bias = GPIO_BIAS_DEFAULT,
        .drive = GPIO_DRIVE_DEFAULT,
        .inverted = false,
        .label = NULL,
    };

    return gpio_open_advanced(gpio, path, line, &config);
}

int gpio_open_name(gpio_t *gpio, const char *path, const char *name, gpio_direction_t direction) {
    gpio_config_t config = {
        .direction = direction,
        .edge = GPIO_EDGE_NONE,
        .bias = GPIO_BIAS_DEFAULT,
        .drive = GPIO_DRIVE_DEFAULT,
        .inverted = false,
        .label = NULL,
    };

    return gpio_open_name_advanced(gpio, path, name, &config);
}

#endif

