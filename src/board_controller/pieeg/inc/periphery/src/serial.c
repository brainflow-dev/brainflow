/*
 * c-periphery
 * https://github.com/vsergeev/c-periphery
 * License: MIT
 */

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>

#include <sys/select.h>
#include <sys/ioctl.h>
#include <termios.h>

#include "serial.h"

struct serial_handle {
    int fd;
    bool use_termios_timeout;

    struct {
        int c_errno;
        char errmsg[96];
    } error;
};

static int _serial_error(serial_t *serial, int code, int c_errno, const char *fmt, ...) {
    va_list ap;

    serial->error.c_errno = c_errno;

    va_start(ap, fmt);
    vsnprintf(serial->error.errmsg, sizeof(serial->error.errmsg), fmt, ap);
    va_end(ap);

    /* Tack on strerror() and errno */
    if (c_errno) {
        char buf[64];
        strerror_r(c_errno, buf, sizeof(buf));
        snprintf(serial->error.errmsg+strlen(serial->error.errmsg), sizeof(serial->error.errmsg)-strlen(serial->error.errmsg), ": %s [errno %d]", buf, c_errno);
    }

    return code;
}

serial_t *serial_new(void) {
    serial_t *serial = calloc(1, sizeof(serial_t));
    if (serial == NULL)
        return NULL;

    serial->fd = -1;

    return serial;
}

void serial_free(serial_t *serial) {
    free(serial);
}

static int _serial_baudrate_to_bits(uint32_t baudrate) {
    switch (baudrate) {
        case 50: return B50;
        case 75: return B75;
        case 110: return B110;
        case 134: return B134;
        case 150: return B150;
        case 200: return B200;
        case 300: return B300;
        case 600: return B600;
        case 1200: return B1200;
        case 1800: return B1800;
        case 2400: return B2400;
        case 4800: return B4800;
        case 9600: return B9600;
        case 19200: return B19200;
        case 38400: return B38400;
        case 57600: return B57600;
        case 115200: return B115200;
        case 230400: return B230400;
        case 460800: return B460800;
        case 500000: return B500000;
        case 576000: return B576000;
        case 921600: return B921600;
        case 1000000: return B1000000;
        case 1152000: return B1152000;
        case 1500000: return B1500000;
        case 2000000: return B2000000;
#ifdef B2500000
        case 2500000: return B2500000;
#endif
#ifdef B3000000
        case 3000000: return B3000000;
#endif
#ifdef B3500000
        case 3500000: return B3500000;
#endif
#ifdef B4000000
        case 4000000: return B4000000;
#endif
        default: return -1;
    }
}

static int _serial_bits_to_baudrate(uint32_t bits) {
    switch (bits) {
        case B0: return 0;
        case B50: return 50;
        case B75: return 75;
        case B110: return 110;
        case B134: return 134;
        case B150: return 150;
        case B200: return 200;
        case B300: return 300;
        case B600: return 600;
        case B1200: return 1200;
        case B1800: return 1800;
        case B2400: return 2400;
        case B4800: return 4800;
        case B9600: return 9600;
        case B19200: return 19200;
        case B38400: return 38400;
        case B57600: return 57600;
        case B115200: return 115200;
        case B230400: return 230400;
        case B460800: return 460800;
        case B500000: return 500000;
        case B576000: return 576000;
        case B921600: return 921600;
        case B1000000: return 1000000;
        case B1152000: return 1152000;
        case B1500000: return 1500000;
        case B2000000: return 2000000;
#ifdef B2500000
        case B2500000: return 2500000;
#endif
#ifdef B3000000
        case B3000000: return 3000000;
#endif
#ifdef B3500000
        case B3500000: return 3500000;
#endif
#ifdef B4000000
        case B4000000: return 4000000;
#endif
        default: return -1;
    }
}

int serial_open(serial_t *serial, const char *path, uint32_t baudrate) {
    return serial_open_advanced(serial, path, baudrate, 8, PARITY_NONE, 1, false, false);
}

int serial_open_advanced(serial_t *serial, const char *path, uint32_t baudrate, unsigned int databits, serial_parity_t parity, unsigned int stopbits, bool xonxoff, bool rtscts) {
    struct termios termios_settings;

    /* Validate args */
    if (databits != 5 && databits != 6 && databits != 7 && databits != 8)
        return _serial_error(serial, SERIAL_ERROR_ARG, 0, "Invalid data bits (can be 5,6,7,8)");
    if (parity != PARITY_NONE && parity != PARITY_ODD && parity != PARITY_EVEN)
        return _serial_error(serial, SERIAL_ERROR_ARG, 0, "Invalid parity (can be PARITY_NONE,PARITY_ODD,PARITY_EVEN)");
    if (stopbits != 1 && stopbits != 2)
        return _serial_error(serial, SERIAL_ERROR_ARG, 0, "Invalid stop bits (can be 1,2)");

    memset(serial, 0, sizeof(serial_t));

    /* Open serial port */
    if ((serial->fd = open(path, O_RDWR | O_NOCTTY)) < 0)
        return _serial_error(serial, SERIAL_ERROR_OPEN, errno, "Opening serial port \"%s\"", path);

    memset(&termios_settings, 0, sizeof(termios_settings));

    /* c_iflag */

    /* Ignore break characters */
    termios_settings.c_iflag = IGNBRK;
    if (parity != PARITY_NONE)
        termios_settings.c_iflag |= INPCK;
    /* Only use ISTRIP when less than 8 bits as it strips the 8th bit */
    if (parity != PARITY_NONE && databits != 8)
        termios_settings.c_iflag |= ISTRIP;
    if (xonxoff)
        termios_settings.c_iflag |= (IXON | IXOFF);

    /* c_oflag */
    termios_settings.c_oflag = 0;

    /* c_lflag */
    termios_settings.c_lflag = 0;

    /* c_cflag */
    /* Enable receiver, ignore modem control lines */
    termios_settings.c_cflag = CREAD | CLOCAL;

    /* Databits */
    if (databits == 5)
        termios_settings.c_cflag |= CS5;
    else if (databits == 6)
        termios_settings.c_cflag |= CS6;
    else if (databits == 7)
        termios_settings.c_cflag |= CS7;
    else if (databits == 8)
        termios_settings.c_cflag |= CS8;

    /* Parity */
    if (parity == PARITY_EVEN)
        termios_settings.c_cflag |= PARENB;
    else if (parity == PARITY_ODD)
        termios_settings.c_cflag |= (PARENB | PARODD);

    /* Stopbits */
    if (stopbits == 2)
        termios_settings.c_cflag |= CSTOPB;

    /* RTS/CTS */
    if (rtscts)
        termios_settings.c_cflag |= CRTSCTS;

    /* Baudrate */
    cfsetispeed(&termios_settings, _serial_baudrate_to_bits(baudrate));
    cfsetospeed(&termios_settings, _serial_baudrate_to_bits(baudrate));

    /* Set termios attributes */
    if (tcsetattr(serial->fd, TCSANOW, &termios_settings) < 0) {
        int errsv = errno;
        close(serial->fd);
        serial->fd = -1;
        return _serial_error(serial, SERIAL_ERROR_CONFIGURE, errsv, "Setting serial port attributes");
    }

    serial->use_termios_timeout = false;

    return 0;
}

int serial_read(serial_t *serial, uint8_t *buf, size_t len, int timeout_ms) {
    ssize_t ret;

    struct timeval tv_timeout;
    tv_timeout.tv_sec = timeout_ms / 1000;
    tv_timeout.tv_usec = (timeout_ms % 1000) * 1000;

    size_t bytes_read = 0;

    while (bytes_read < len) {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(serial->fd, &rfds);

        if ((ret = select(serial->fd+1, &rfds, NULL, NULL, (timeout_ms < 0) ? NULL : &tv_timeout)) < 0)
            return _serial_error(serial, SERIAL_ERROR_IO, errno, "select() on serial port");

        /* Timeout */
        if (ret == 0)
            break;

        if ((ret = read(serial->fd, buf + bytes_read, len - bytes_read)) < 0)
            return _serial_error(serial, SERIAL_ERROR_IO, errno, "Reading serial port");

        /* If we're using VMIN or VMIN+VTIME semantics for end of read, return now */
        if (serial->use_termios_timeout)
            return ret;

        /* Empty read */
        if (ret == 0 && len != 0)
            return _serial_error(serial, SERIAL_ERROR_IO, 0, "Reading serial port: unexpected empty read");

        bytes_read += ret;
    }

    return bytes_read;
}

int serial_write(serial_t *serial, const uint8_t *buf, size_t len) {
    ssize_t ret;

    if ((ret = write(serial->fd, buf, len)) < 0)
        return _serial_error(serial, SERIAL_ERROR_IO, errno, "Writing serial port");

    return ret;
}

int serial_flush(serial_t *serial) {

    if (tcdrain(serial->fd) < 0)
        return _serial_error(serial, SERIAL_ERROR_IO, errno, "Flushing serial port");

    return 0;
}

int serial_input_waiting(serial_t *serial, unsigned int *count) {
    if (ioctl(serial->fd, TIOCINQ, count) < 0)
        return _serial_error(serial, SERIAL_ERROR_IO, errno, "TIOCINQ query");

    return 0;
}

int serial_output_waiting(serial_t *serial, unsigned int *count) {
    if (ioctl(serial->fd, TIOCOUTQ, count) < 0)
        return _serial_error(serial, SERIAL_ERROR_IO, errno, "TIOCOUTQ query");

    return 0;
}

int serial_poll(serial_t *serial, int timeout_ms) {
    struct pollfd fds[1];
    int ret;

    /* Poll */
    fds[0].fd = serial->fd;
    fds[0].events = POLLIN | POLLPRI;
    if ((ret = poll(fds, 1, timeout_ms)) < 0)
        return _serial_error(serial, SERIAL_ERROR_IO, errno, "Polling serial port");

    if (ret)
        return 1;

    /* Timed out */
    return 0;
}

int serial_close(serial_t *serial) {
    if (serial->fd < 0)
        return 0;

    if (close(serial->fd) < 0)
        return _serial_error(serial, SERIAL_ERROR_CLOSE, errno, "Closing serial port");

    serial->fd = -1;

    return 0;
}

int serial_get_baudrate(serial_t *serial, uint32_t *baudrate) {
    struct termios termios_settings;

    if (tcgetattr(serial->fd, &termios_settings) < 0)
        return _serial_error(serial, SERIAL_ERROR_QUERY, errno, "Getting serial port attributes");

    *baudrate = _serial_bits_to_baudrate(cfgetospeed(&termios_settings));

    return 0;
}

int serial_get_databits(serial_t *serial, unsigned int *databits) {
    struct termios termios_settings;

    if (tcgetattr(serial->fd, &termios_settings) < 0)
        return _serial_error(serial, SERIAL_ERROR_QUERY, errno, "Getting serial port attributes");

    switch (termios_settings.c_cflag & CSIZE) {
        case CS5:
            *databits = 5;
            break;
        case CS6:
            *databits = 6;
            break;
        case CS7:
            *databits = 7;
            break;
        case CS8:
            *databits = 8;
            break;
    }

    return 0;
}

int serial_get_parity(serial_t *serial, serial_parity_t *parity) {
    struct termios termios_settings;

    if (tcgetattr(serial->fd, &termios_settings) < 0)
        return _serial_error(serial, SERIAL_ERROR_QUERY, errno, "Getting serial port attributes");

    if ((termios_settings.c_cflag & PARENB) == 0)
        *parity = PARITY_NONE;
    else if ((termios_settings.c_cflag & PARODD) == 0)
        *parity = PARITY_EVEN;
    else
        *parity = PARITY_ODD;

    return 0;
}

int serial_get_stopbits(serial_t *serial, unsigned int *stopbits) {
    struct termios termios_settings;

    if (tcgetattr(serial->fd, &termios_settings) < 0)
        return _serial_error(serial, SERIAL_ERROR_QUERY, errno, "Getting serial port attributes");

    if (termios_settings.c_cflag & CSTOPB)
        *stopbits = 2;
    else
        *stopbits = 1;

    return 0;
}

int serial_get_xonxoff(serial_t *serial, bool *xonxoff) {
    struct termios termios_settings;

    if (tcgetattr(serial->fd, &termios_settings) < 0)
        return _serial_error(serial, SERIAL_ERROR_QUERY, errno, "Getting serial port attributes");

    if (termios_settings.c_iflag & (IXON | IXOFF))
        *xonxoff = true;
    else
        *xonxoff = false;

    return 0;
}

int serial_get_rtscts(serial_t *serial, bool *rtscts) {
    struct termios termios_settings;

    if (tcgetattr(serial->fd, &termios_settings) < 0)
        return _serial_error(serial, SERIAL_ERROR_QUERY, errno, "Getting serial port attributes");

    if (termios_settings.c_cflag & CRTSCTS)
        *rtscts = true;
    else
        *rtscts = false;

    return 0;
}

int serial_get_vmin(serial_t *serial, unsigned int *vmin) {
    struct termios termios_settings;

    if (tcgetattr(serial->fd, &termios_settings) < 0)
        return _serial_error(serial, SERIAL_ERROR_QUERY, errno, "Getting serial port attributes");

    *vmin = termios_settings.c_cc[VMIN];

    return 0;
}

int serial_get_vtime(serial_t *serial, float *vtime) {
    struct termios termios_settings;

    if (tcgetattr(serial->fd, &termios_settings) < 0)
        return _serial_error(serial, SERIAL_ERROR_QUERY, errno, "Getting serial port attributes");

    *vtime = ((float)termios_settings.c_cc[VTIME]) / 10;

    return 0;
}

int serial_set_baudrate(serial_t *serial, uint32_t baudrate) {
    struct termios termios_settings;

    if (tcgetattr(serial->fd, &termios_settings) < 0)
        return _serial_error(serial, SERIAL_ERROR_QUERY, errno, "Getting serial port attributes");

    cfsetispeed(&termios_settings, _serial_baudrate_to_bits(baudrate));
    cfsetospeed(&termios_settings, _serial_baudrate_to_bits(baudrate));

    if (tcsetattr(serial->fd, TCSANOW, &termios_settings) < 0)
        return _serial_error(serial, SERIAL_ERROR_CONFIGURE, errno, "Setting serial port attributes");

    return 0;
}

int serial_set_databits(serial_t *serial, unsigned int databits) {
    struct termios termios_settings;

    if (databits != 5 && databits != 6 && databits != 7 && databits != 8)
        return _serial_error(serial, SERIAL_ERROR_ARG, 0, "Invalid data bits (can be 5,6,7,8)");

    if (tcgetattr(serial->fd, &termios_settings) < 0)
        return _serial_error(serial, SERIAL_ERROR_QUERY, errno, "Getting serial port attributes");

    termios_settings.c_cflag &= ~CSIZE;
    if (databits == 5)
        termios_settings.c_cflag |= CS5;
    else if (databits == 6)
        termios_settings.c_cflag |= CS6;
    else if (databits == 7)
        termios_settings.c_cflag |= CS7;
    else if (databits == 8)
        termios_settings.c_cflag |= CS8;

    if (tcsetattr(serial->fd, TCSANOW, &termios_settings) < 0)
        return _serial_error(serial, SERIAL_ERROR_CONFIGURE, errno, "Setting serial port attributes");

    return 0;
}

int serial_set_parity(serial_t *serial, enum serial_parity parity) {
    struct termios termios_settings;

    if (parity != PARITY_NONE && parity != PARITY_ODD && parity != PARITY_EVEN)
        return _serial_error(serial, SERIAL_ERROR_ARG, 0, "Invalid parity (can be PARITY_NONE,PARITY_ODD,PARITY_EVEN)");

    if (tcgetattr(serial->fd, &termios_settings) < 0)
        return _serial_error(serial, SERIAL_ERROR_QUERY, errno, "Getting serial port attributes");

    termios_settings.c_iflag &= ~(INPCK | ISTRIP);
    if (parity != PARITY_NONE)
        termios_settings.c_iflag |= (INPCK | ISTRIP);

    termios_settings.c_cflag &= ~(PARENB | PARODD);
    if (parity == PARITY_EVEN)
        termios_settings.c_cflag |= PARENB;
    else if (parity == PARITY_ODD)
        termios_settings.c_cflag |= (PARENB | PARODD);

    if (tcsetattr(serial->fd, TCSANOW, &termios_settings) < 0)
        return _serial_error(serial, SERIAL_ERROR_CONFIGURE, errno, "Setting serial port attributes");

    return 0;
}

int serial_set_stopbits(serial_t *serial, unsigned int stopbits) {
    struct termios termios_settings;

    if (stopbits != 1 && stopbits != 2)
        return _serial_error(serial, SERIAL_ERROR_ARG, 0, "Invalid stop bits (can be 1,2)");

    if (tcgetattr(serial->fd, &termios_settings) < 0)
        return _serial_error(serial, SERIAL_ERROR_QUERY, errno, "Getting serial port attributes");

    termios_settings.c_cflag &= ~(CSTOPB);
    if (stopbits == 2)
        termios_settings.c_cflag |= CSTOPB;

    if (tcsetattr(serial->fd, TCSANOW, &termios_settings) < 0)
        return _serial_error(serial, SERIAL_ERROR_CONFIGURE, errno, "Setting serial port attributes");

    return 0;
}

int serial_set_xonxoff(serial_t *serial, bool enabled) {
    struct termios termios_settings;

    if (tcgetattr(serial->fd, &termios_settings) < 0)
        return _serial_error(serial, SERIAL_ERROR_QUERY, errno, "Getting serial port attributes");

    termios_settings.c_iflag &= ~(IXON | IXOFF | IXANY);
    if (enabled)
        termios_settings.c_iflag |= (IXON | IXOFF);

    if (tcsetattr(serial->fd, TCSANOW, &termios_settings) < 0)
        return _serial_error(serial, SERIAL_ERROR_CONFIGURE, errno, "Setting serial port attributes");

    return 0;
}

int serial_set_rtscts(serial_t *serial, bool enabled) {
    struct termios termios_settings;

    if (tcgetattr(serial->fd, &termios_settings) < 0)
        return _serial_error(serial, SERIAL_ERROR_QUERY, errno, "Getting serial port attributes");

    termios_settings.c_cflag &= ~CRTSCTS;
    if (enabled)
        termios_settings.c_cflag |= CRTSCTS;

    if (tcsetattr(serial->fd, TCSANOW, &termios_settings) < 0)
        return _serial_error(serial, SERIAL_ERROR_CONFIGURE, errno, "Setting serial port attributes");

    return 0;
}

int serial_set_vmin(serial_t *serial, unsigned int vmin) {
    struct termios termios_settings;

    if (vmin > 255)
        return _serial_error(serial, SERIAL_ERROR_ARG, 0, "Invalid vmin (can be 0-255)");

    if (tcgetattr(serial->fd, &termios_settings) < 0)
        return _serial_error(serial, SERIAL_ERROR_QUERY, errno, "Getting serial port attributes");

    termios_settings.c_cc[VMIN] = vmin;

    if (tcsetattr(serial->fd, TCSANOW, &termios_settings) < 0)
        return _serial_error(serial, SERIAL_ERROR_CONFIGURE, errno, "Setting serial port attributes");

    serial->use_termios_timeout = vmin > 0;

    return 0;
}

int serial_set_vtime(serial_t *serial, float vtime) {
    struct termios termios_settings;

    if (vtime < 0.0 || vtime > 25.5)
        return _serial_error(serial, SERIAL_ERROR_ARG, 0, "Invalid vtime (can be 0-25.5)");

    if (tcgetattr(serial->fd, &termios_settings) < 0)
        return _serial_error(serial, SERIAL_ERROR_QUERY, errno, "Getting serial port attributes");

    termios_settings.c_cc[VTIME] = ((unsigned int)(vtime * 10));

    if (tcsetattr(serial->fd, TCSANOW, &termios_settings) < 0)
        return _serial_error(serial, SERIAL_ERROR_CONFIGURE, errno, "Setting serial port attributes");

    return 0;
}

int serial_tostring(serial_t *serial, char *str, size_t len) {
    struct termios termios_settings;
    uint32_t baudrate;
    const char *databits_str, *parity_str, *stopbits_str, *xonxoff_str, *rtscts_str;
    unsigned int vmin;
    float vtime;

    /* Instead of calling all of our individual getter functions, let's poll
     * termios attributes once to be efficient. */

    if (tcgetattr(serial->fd, &termios_settings) < 0)
        return snprintf(str, len, "Serial (baudrate=?, databits=?, parity=?, stopbits=?, xonxoff=?, rtscts=?)");

     baudrate = _serial_bits_to_baudrate(cfgetospeed(&termios_settings));

     switch (termios_settings.c_cflag & CSIZE) {
        case CS5: databits_str = "5"; break;
        case CS6: databits_str = "6"; break;
        case CS7: databits_str = "7"; break;
        case CS8: databits_str = "8"; break;
        default: databits_str = "?";
    }

    if ((termios_settings.c_cflag & PARENB) == 0)
        parity_str = "none";
    else if ((termios_settings.c_cflag & PARODD) == 0)
        parity_str = "even";
    else
        parity_str = "odd";

    if (termios_settings.c_cflag & CSTOPB)
        stopbits_str = "2";
    else
        stopbits_str = "1";

    if (termios_settings.c_iflag & (IXON | IXOFF))
        xonxoff_str = "true";
    else
        xonxoff_str = "false";

    if (termios_settings.c_cflag & CRTSCTS)
        rtscts_str = "true";
    else
        rtscts_str = "false";

    vmin = termios_settings.c_cc[VMIN];
    vtime = ((float)termios_settings.c_cc[VTIME]) / 10;

    return snprintf(str, len, "Serial (fd=%d, baudrate=%u, databits=%s, parity=%s, stopbits=%s, xonxoff=%s, rtscts=%s, vmin=%u, vtime=%.1f)",
                    serial->fd, baudrate, databits_str, parity_str, stopbits_str, xonxoff_str, rtscts_str, vmin, vtime);
}

const char *serial_errmsg(serial_t *serial) {
    return serial->error.errmsg;
}

int serial_errno(serial_t *serial) {
    return serial->error.c_errno;
}

int serial_fd(serial_t *serial) {
    return serial->fd;
}

