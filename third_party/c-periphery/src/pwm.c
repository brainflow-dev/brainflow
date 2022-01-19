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

#include <inttypes.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

#include "pwm.h"

#define P_PATH_MAX  256
/* Delay between checks for successful PWM export (100ms) */
#define PWM_EXPORT_STAT_DELAY       100000
/* Number of retries to check for successful PWM exports */
#define PWM_EXPORT_STAT_RETRIES     10

struct pwm_handle {
    unsigned int chip;
    unsigned int channel;
    uint64_t period_ns;

    struct {
        int c_errno;
        char errmsg[96];
    } error;
};

static int _pwm_error(pwm_t *pwm, int code, int c_errno, const char *fmt, ...) {
    va_list ap;

    pwm->error.c_errno = c_errno;

    va_start(ap, fmt);
    vsnprintf(pwm->error.errmsg, sizeof(pwm->error.errmsg), fmt, ap);
    va_end(ap);

    /* Tack on strerror() and errno */
    if (c_errno) {
        char buf[64];
        strerror_r(c_errno, buf, sizeof(buf));
        snprintf(pwm->error.errmsg+strlen(pwm->error.errmsg), sizeof(pwm->error.errmsg)-strlen(pwm->error.errmsg), ": %s [errno %d]", buf, c_errno);
    }

    return code;
}

pwm_t *pwm_new(void) {
    pwm_t *pwm = calloc(1, sizeof(pwm_t));
    if (pwm == NULL)
        return NULL;

    pwm->chip = -1;
    pwm->channel = -1;

    return pwm;
}

int pwm_open(pwm_t *pwm, unsigned int chip, unsigned int channel) {
    char channel_path[P_PATH_MAX];
    struct stat stat_buf;
    int ret;

    snprintf(channel_path, sizeof(channel_path), "/sys/class/pwm/pwmchip%u/pwm%u", chip, channel);

    /* Check if PWM channel exists */
    if (stat(channel_path, &stat_buf) < 0) {
        char path[P_PATH_MAX];
        char buf[16];
        int fd, len;

        /* Export the PWM channel */
        snprintf(path, sizeof(path), "/sys/class/pwm/pwmchip%u/export", chip);

        len = snprintf(buf, sizeof(buf), "%u\n", channel);

        if ((fd = open(path, O_WRONLY)) < 0)
            return _pwm_error(pwm, PWM_ERROR_OPEN, errno, "Opening PWM: opening 'export'");

        if (write(fd, buf, len) < 0) {
            int errsv = errno;
            close(fd);
            return _pwm_error(pwm, PWM_ERROR_OPEN, errsv, "Opening PWM: writing 'export'");
        }

        if (close(fd) < 0)
            return _pwm_error(pwm, PWM_ERROR_OPEN, errno, "Opening PWM: closing 'export'");

        /* Wait until PWM channel appears */
        unsigned int retry_count;
        for (retry_count = 0; retry_count < PWM_EXPORT_STAT_RETRIES; retry_count++) {
            if ((ret = stat(path, &stat_buf)) < 0 && errno != ENOENT)
                return _pwm_error(pwm, PWM_ERROR_OPEN, errno, "Opening PWM: stat 'pwm%u/' after export", channel);
            else if (ret == 0)
                break;

            usleep(PWM_EXPORT_STAT_DELAY);
        }

        if (retry_count == PWM_EXPORT_STAT_RETRIES)
            return _pwm_error(pwm, PWM_ERROR_OPEN, 0, "Opening PWM: waiting for 'pwm%u/' timed out", channel);

        snprintf(path, sizeof(path), "/sys/class/pwm/pwmchip%u/pwm%u/period", chip, channel);

        /* Loop until period is writable. This could take some time after
         * export as application of udev rules after export is asynchronous. */
        for (retry_count = 0; retry_count < PWM_EXPORT_STAT_RETRIES; retry_count++) {
            if ((fd = open(path, O_WRONLY)) < 0) {
                if (errno != EACCES || (errno == EACCES && retry_count == PWM_EXPORT_STAT_RETRIES - 1))
                    return _pwm_error(pwm, PWM_ERROR_OPEN, errno, "Opening PWM: opening 'pwm%u/period' after export", channel);
            } else {
                close(fd);
                break;
            }

            usleep(PWM_EXPORT_STAT_DELAY);
        }
    }

    memset(pwm, 0, sizeof(pwm_t));
    pwm->chip = chip;
    pwm->channel = channel;

    ret = pwm_get_period_ns(pwm, &pwm->period_ns);
    if (ret < 0)
        return ret;

    return 0;
}

int pwm_enable(pwm_t *pwm) {
    return pwm_set_enabled(pwm, true);
}

int pwm_disable(pwm_t *pwm) {
    return pwm_set_enabled(pwm, false);
}

int pwm_close(pwm_t *pwm) {
    char path[P_PATH_MAX];
    char buf[16];
    int len;
    int fd;

    if (pwm->channel == ((unsigned int) -1))
        return 0;

    /* Unexport the PWM */
    snprintf(path, sizeof(path), "/sys/class/pwm/pwmchip%u/unexport", pwm->chip);

    len = snprintf(buf, sizeof(buf), "%u\n", pwm->channel);

    if ((fd = open(path, O_WRONLY)) < 0)
        return _pwm_error(pwm, PWM_ERROR_CLOSE, errno, "Closing PWM: opening 'unexport'");

    if (write(fd, buf, len) < 0) {
        int errsv = errno;
        close(fd);
        return _pwm_error(pwm, PWM_ERROR_CLOSE, errsv, "Closing PWM: writing 'unexport'");
    }

    if (close(fd) < 0)
        return _pwm_error(pwm, PWM_ERROR_CLOSE, errno, "Closing PWM: closing 'unexport'");

    pwm->chip = -1;
    pwm->channel = -1;

    return 0;
}

void pwm_free(pwm_t *pwm) {
    free(pwm);
}

static int pwm_read_attribute(pwm_t *pwm, const char *name, char *buf, size_t len) {
    char path[P_PATH_MAX];
    int fd, ret;

    snprintf(path, sizeof(path), "/sys/class/pwm/pwmchip%u/pwm%u/%s", pwm->chip, pwm->channel, name);

    if ((fd = open(path, O_RDONLY)) < 0)
        return _pwm_error(pwm, PWM_ERROR_QUERY, errno, "Opening PWM '%s'", name);

    if ((ret = read(fd, buf, len)) < 0) {
        int errsv = errno;
        close(fd);
        return _pwm_error(pwm, PWM_ERROR_QUERY, errsv, "Reading PWM '%s'", name);
    }

    if (close(fd) < 0)
        return _pwm_error(pwm, PWM_ERROR_QUERY, errno, "Closing PWM '%s'", name);

    buf[ret] = '\0';

    return 0;
}

static int pwm_write_attribute(pwm_t *pwm, const char *name, const char *buf, size_t len) {
    char path[P_PATH_MAX];
    int fd;

    snprintf(path, sizeof(path), "/sys/class/pwm/pwmchip%u/pwm%u/%s", pwm->chip, pwm->channel, name);

    if ((fd = open(path, O_WRONLY)) < 0)
        return _pwm_error(pwm, PWM_ERROR_CONFIGURE, errno, "Opening PWM '%s'", name);

    if (write(fd, buf, len) < 0) {
        int errsv = errno;
        close(fd);
        return _pwm_error(pwm, PWM_ERROR_CONFIGURE, errsv, "Writing PWM '%s'", name);
    }

    if (close(fd) < 0)
        return _pwm_error(pwm, PWM_ERROR_CONFIGURE, errno, "Closing PWM '%s'", name);

    return 0;
}

int pwm_get_enabled(pwm_t *pwm, bool *enabled) {
    char buf[2];
    int ret;

    if ((ret = pwm_read_attribute(pwm, "enable", buf, sizeof(buf))) < 0)
        return ret;

    if (buf[0] == '0')
        *enabled = false;
    else if (buf[0] == '1')
        *enabled = true;
    else
        return _pwm_error(pwm, PWM_ERROR_QUERY, errno, "Unknown PWM 'enabled' value");

    return 0;
}

int pwm_get_period_ns(pwm_t *pwm, uint64_t *period_ns) {
    char buf[32];
    int ret;
    uint64_t value;

    if ((ret = pwm_read_attribute(pwm, "period", buf, sizeof(buf))) < 0)
        return ret;

    errno = 0;
    value = strtoul(buf, NULL, 10);
    if (errno != 0)
        return _pwm_error(pwm, PWM_ERROR_QUERY, errno, "Unknown PWM 'period' value");

    /* Cache the period for fast duty cycle updates */
    pwm->period_ns = value;

    *period_ns = value;

    return 0;
}

int pwm_get_duty_cycle_ns(pwm_t *pwm, uint64_t *duty_cycle_ns) {
    char buf[32];
    int ret;
    uint64_t value;

    if ((ret = pwm_read_attribute(pwm, "duty_cycle", buf, sizeof(buf))) < 0)
        return ret;

    errno = 0;
    value = strtoul(buf, NULL, 10);
    if (errno != 0)
        return _pwm_error(pwm, PWM_ERROR_QUERY, errno, "Unknown PWM 'duty_cycle' value");

    *duty_cycle_ns = value;

    return 0;
}

int pwm_get_period(pwm_t *pwm, double *period) {
    int ret;
    uint64_t period_ns;

    if ((ret = pwm_get_period_ns(pwm, &period_ns)) < 0)
        return ret;

    *period = ((double) period_ns) / 1e9;

    return 0;
}

int pwm_get_duty_cycle(pwm_t *pwm, double *duty_cycle) {
    int ret;
    uint64_t duty_cycle_ns;

    if ((ret = pwm_get_duty_cycle_ns(pwm, &duty_cycle_ns)) < 0)
        return ret;

    *duty_cycle = ((double) duty_cycle_ns) / ((double) pwm->period_ns);

    return 0;
}

int pwm_get_frequency(pwm_t *pwm, double *frequency) {
    int ret;
    uint64_t period_ns;

    if ((ret = pwm_get_period_ns(pwm, &period_ns)) < 0)
        return ret;

    *frequency = 1e9 / ((double) period_ns);

    return 0;
}

int pwm_get_polarity(pwm_t *pwm, pwm_polarity_t *polarity) {
    int ret;
    char buf[16];

    if ((ret = pwm_read_attribute(pwm, "polarity", buf, sizeof(buf))) < 0)
        return ret;

    if (strcmp(buf, "normal\n") == 0)
        *polarity = PWM_POLARITY_NORMAL;
    else if (strcmp(buf, "inversed\n") == 0)
        *polarity = PWM_POLARITY_INVERSED;
    else
        return _pwm_error(pwm, PWM_ERROR_QUERY, errno, "Unknown PWM 'polarity' value");

    return 0;
}

int pwm_set_enabled(pwm_t *pwm, bool enabled) {
    return pwm_write_attribute(pwm, "enable", enabled ? "1\n" : "0\n", 2);
}

int pwm_set_period_ns(pwm_t *pwm, uint64_t period_ns) {
    char buf[32];
    int len;
    int ret;

    len = snprintf(buf, sizeof(buf), "%" PRId64 "\n", period_ns);

    if ((ret = pwm_write_attribute(pwm, "period", buf, len)) < 0)
        return ret;

    /* Cache the period for fast duty cycle updates */
    pwm->period_ns = period_ns;

    return 0;
}

int pwm_set_duty_cycle_ns(pwm_t *pwm, uint64_t duty_cycle_ns) {
    char buf[32];
    int len;

    len = snprintf(buf, sizeof(buf), "%" PRId64 "\n", duty_cycle_ns);

    return pwm_write_attribute(pwm, "duty_cycle", buf, len);
}

int pwm_set_period(pwm_t *pwm, double period) {
    uint64_t period_ns = (uint64_t)(period * 1e9);

    return pwm_set_period_ns(pwm, period_ns);
}

int pwm_set_duty_cycle(pwm_t *pwm, double duty_cycle) {
    uint64_t duty_cycle_ns;

    if (duty_cycle < 0 || duty_cycle > 1)
        return _pwm_error(pwm, PWM_ERROR_ARG, 0, "PWM duty cycle out of bounds (should be between 0.0 and 1.0)");

    duty_cycle_ns = (uint64_t)(((double) pwm->period_ns) * duty_cycle);

    return pwm_set_duty_cycle_ns(pwm, duty_cycle_ns);
}

int pwm_set_frequency(pwm_t *pwm, double frequency) {
    uint64_t period_ns = (uint64_t)(1e9 / frequency);

    return pwm_set_period_ns(pwm, period_ns);
}

int pwm_set_polarity(pwm_t *pwm, pwm_polarity_t polarity) {
    const char *buf;

    if (polarity == PWM_POLARITY_NORMAL)
        buf = "normal\n";
    else if (polarity == PWM_POLARITY_INVERSED)
        buf = "inversed\n";
    else
        return _pwm_error(pwm, PWM_ERROR_ARG, 0, "Invalid PWM polarity (can be normal, inversed)");

    return pwm_write_attribute(pwm, "polarity", buf, strlen(buf));
}

unsigned int pwm_chip(pwm_t *pwm) {
    return pwm->chip;
}

unsigned int pwm_channel(pwm_t *pwm) {
    return pwm->channel;
}

int pwm_tostring(pwm_t *pwm, char *str, size_t len) {
    double period;
    char period_str[16];
    double duty_cycle;
    char duty_cycle_str[16];
    pwm_polarity_t polarity;
    const char *polarity_str;
    bool enabled;
    const char *enabled_str;

    if (pwm_get_period(pwm, &period) < 0)
        strcpy(period_str, "<error>");
    else
        snprintf(period_str, sizeof(period_str), "%f", period);

    if (pwm_get_duty_cycle(pwm, &duty_cycle) < 0)
        strcpy(duty_cycle_str, "<error>");
    else
        snprintf(duty_cycle_str, sizeof(duty_cycle_str), "%f", duty_cycle);

    if (pwm_get_polarity(pwm, &polarity) < 0)
        polarity_str = "<error>";
    else
        polarity_str = (polarity == PWM_POLARITY_NORMAL) ? "normal" :
                       (polarity == PWM_POLARITY_INVERSED) ? "inversed" : "unknown";

    if (pwm_get_enabled(pwm, &enabled) < 0)
        enabled_str = "<error>";
    else
        enabled_str = enabled ? "true" : "false";

    return snprintf(str, len, "PWM %u, chip %u (period=%s sec, duty_cycle=%s%%, polarity=%s, enabled=%s)", pwm->channel, pwm->chip, period_str, duty_cycle_str, polarity_str, enabled_str);
}

int pwm_errno(pwm_t *pwm) {
    return pwm->error.c_errno;
}

const char *pwm_errmsg(pwm_t *pwm) {
    return pwm->error.errmsg;
}
