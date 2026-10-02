// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#if __has_include(<termios.h>) && !defined(_WIN32)
#include <cpr/strchar.h>
#include <string.h>
#include <stdlib.h>
#include <poll.h>
#include <poll.h>
#include <fcntl.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include "tty.h"

static bool tty_setup(tty_ctx *tty) {
    if (!tty) return false;
    long ioflags = fcntl(tty->fd, F_GETFL);
    tcgetattr(tty->fd, &tty->active);
    tcgetattr(tty->fd, &tty->saved);
    tty_reset(tty);
    fcntl(tty->fd, F_SETFL, ioflags & ~O_NDELAY); // NOLINT
    tty->error = 0;
    tty->packet = 0;
    tty->nl1 = tty->nl2 = 0;

#if defined(TIOCM_RTS) && defined(TIOCMODG)
    int mcs = 0;
    ioctl(tty->fd, TIOCMODG, &mcs);
    mcs |= TIOCM_RTS;
    ioctl(tty->fd, TIOCMODS, &mcs);
#endif
    return true;
}

bool tty_ispath(const char *path) {
    if (!path) return false;
    if (eq_n(path, "tty", 3) && !strchr(path, '/')) return true;
    if (eq_n(path, "/dev/tty", 8) && !strchr(path + 8, '/')) return true;
    return false;
}

bool tty_echo(tty_ctx *tty, bool echo) {
    if (!tty || !tty->fd) return false;
    if (echo)
        tty->active.c_lflag |= ECHO; // NOLINT
    else
        tty->active.c_lflag &= ~ECHO; // NOLINT
    return tcsetattr(tty->fd, TCSANOW, &tty->active) == 0;
}

bool tty_shell(tty_ctx *tty, bool enable) {
    if (!tty || !tty->fd) return false;
    if (enable)
        tty->active.c_lflag |= ICANON | ECHO | ECHOE | ECHOK | ECHOCTL; // NOLINT
    else
        tty->active.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHOCTL); // NOLINT
    return tcsetattr(tty->fd, TCSANOW, &tty->active) == 0;
}

bool tty_reset(tty_ctx *tty) {
    if (!tty || !tty->fd) return false;
    tty->active.c_oflag = tty->active.c_lflag = 0;
    tty->active.c_cflag = CLOCAL | CREAD | HUPCL; // NOLINT
    tty->active.c_iflag = IGNBRK;

    memset(&tty->active.c_cc, 0, sizeof(tty->active.c_cc));
    tty->active.c_cc[VMIN] = 1;

    tty->active.c_cflag |= tty->saved.c_cflag & (CRTSCTS | CSIZE | PARENB | PARODD | CSTOPB); // NOLINT
    tty->active.c_iflag |= tty->saved.c_iflag & (IXON | IXANY | IXOFF);                       // NOLINT
    tty->packet = 0;
    tty->nl1 = tty->nl2 = 0;
    return tcsetattr(tty->fd, TCSANOW, &tty->active) == 0;
}

bool tty_softfc(tty_ctx *tty, bool enable) {
    if (!tty) return false;
    if (enable)
        tty->active.c_iflag |= (IXON | IXOFF | IXANY); // NOLINT
    else
        tty->active.c_iflag &= ~(IXON | IXOFF | IXANY); // NOLINT
    return tcsetattr(tty->fd, TCSANOW, &tty->active) == 0;
}

bool tty_hardfc(tty_ctx *tty, bool enable) {
    if (!tty) return false;
    if (enable)
        tty->active.c_cflag |= CRTSCTS;
    else
        tty->active.c_cflag &= ~CRTSCTS;
    return tcsetattr(tty->fd, TCSANOW, &tty->active) == 0;
}

bool tty_speed(tty_ctx *tty, long speed) {
    if (!tty) return false;
    if (!speed) return true;

    unsigned long rate = 0;
    switch (speed) {
#ifdef B256000
    case 256000L:
        rate = B256000;
        break;
#endif
#ifdef B230400
    case 230400L:
        rate = B230400;
        break;
#endif
#ifdef B128000
    case 128000L:
        rate = B128000;
        break;
#endif
#ifdef B57600
    case 57600L:
        rate = B57600;
        break;
#endif
#ifdef B56000
    case 56000L:
        rate = B56000;
        break;
#endif
#ifdef B38400
    case 38400L:
        rate = B38400;
        break;
#endif
    case 19200:
        rate = B19200;
        break;
    case 9600:
        rate = B9600;
        break;
    case 4800:
        rate = B4800;
        break;
    case 2400:
        rate = B2400;
        break;
    case 1200:
        rate = B1200;
        break;
    case 600:
        rate = B600;
        break;
    case 300:
        rate = B300;
        break;
    case 110:
        rate = B110;
        break;
    default:
        return false;
    }
    cfsetispeed(&tty->active, rate);
    cfsetospeed(&tty->active, rate);
    return tcsetattr(tty->fd, TCSANOW, &tty->active) == 0;
}

bool tty_format(tty_ctx *tty, const char *format) {
    if (!tty) return false;
    if (!format) return true;
    size_t size = cpr_strlen(format, 5);
    unsigned pos = 0;
    unsigned bits = 8;
    unsigned stop = 1;
    char parity = 'n';

    if (size > pos && strchr("5678", format[pos]))
        bits = format[pos++] - '0';

    if (size > pos && strchr("oOeEnN", format[pos]))
        parity = format[pos++];

    if (size > pos && strchr("12", format[pos]))
        stop = format[pos] - '0';

    if (size > pos) return false;
    tty->active.c_cflag &= ~(PARENB | PARODD | CSTOPB | CSIZE); // NOLINT
    switch (bits) {
    case '5':
        tty->active.c_cflag |= CS5;
        break;
    case '6':
        tty->active.c_cflag |= CS6;
        break;
    case '7':
        tty->active.c_cflag |= CS7;
        break;
    case '8':
        tty->active.c_cflag |= CS8;
        break;
    default:
        return false;
    }

    switch (stop) {
    case '1':
        break;
    case '2':
        tty->active.c_cflag |= CSTOPB;
        break;
    default:
        return false;
    }

    switch (parity) {
    case 'o':
    case 'O':
        tty->active.c_cflag |= (PARENB | PARODD); // NOLINT
        break;
    case 'e':
    case 'E':
        tty->active.c_cflag |= PARENB;
        break;
    case 'n':
    case 'N':
        break;
    default:
        return false;
    }
    return tcsetattr(tty->fd, TCSANOW, &tty->active) == 0;
}

bool tty_open(tty_ctx *tty, const char *path) {
    if (!tty) return false;
    tty->error = EINVAL;
    if (tty_ispath(path))
        tty->fd = open(path, O_RDWR | O_NDELAY);
    else if (eq(path, "-"))
        tty->fd = 0;
    else
        return false;
    if (tty->fd < 0) {
        tty->error = errno;
        return false;
    }
    tty->error = 0;
    return tty_setup(tty);
}

bool tty_sync(tty_ctx *tty) {
    if (!tty) return false;
    if (tty->fd < 0) return false;
    tcdrain(tty->fd);
    return true;
}

bool tty_close(tty_ctx *tty) {
    if (!tty) return false;
    if (tty->fd < 0) return false;
    if (tty->fd > 2) tcflush(tty->fd, TCIOFLUSH);
    tcsetattr(tty->fd, TCSANOW, &tty->saved);
    if (tty->fd > 2) close(tty->fd);
    return true;
}

FILE *tty_file(tty_ctx *tty, const char *mode) {
    if (!tty) return NULL;
    if (tty->fd < 2) return NULL;
    return fdopen(tty->fd, mode);
}

int tty_size(tty_ctx *tty) {
    if (!tty || !tty->fd) return 0;
#ifdef _PC_MAX_CANON
    return (int)fpathconf(tty->fd, _PC_MAX_CANON);
#else
    return MAX_CANON
#endif
}

int tty_line(tty_ctx *tty, const char *nl) {
    if (!tty || !tty->fd) return 0;
    tty->nl1 = tty->nl2 = 0;
    if (!nl) return 0;
    if (*nl) {
        tty->nl1 = nl[0];
        tty->nl2 = nl[1];
    }

    struct termios *attr = &tty->active;
    attr->c_cc[VMIN] = tty->nl1 ? 0 : 1;
    attr->c_cc[VTIME] = 0;
    attr->c_cc[VEOL] = tty->nl1;
    attr->c_cc[VEOL2] = tty->nl2;
    attr->c_lflag |= ICANON;
    tcsetattr(tty->fd, TCSANOW, attr);
    return tty_size(tty);
}

int tty_packet(tty_ctx *tty, uint8_t size, uint8_t timer) {
    if (!tty || !tty->fd) return 0;
#ifdef _PC_MAX_INPUT
    long max = fpathconf(tty->fd, _PC_MAX_INPUT);
#else
    long max = MAX_INPUT;
#endif
    if (size > (size_t)max) return 0;

    struct termios *attr = &tty->active;
    attr->c_cc[VEOL] = attr->c_cc[VEOL2] = 0;
    attr->c_cc[VMIN] = size;
    attr->c_cc[VTIME] = timer;
    attr->c_lflag &= ~ICANON; // NOLINT
    tcsetattr(tty->fd, TCSANOW, attr);
    return (int)size;
}

size_t tty_read(tty_ctx *tty, void *data, size_t size) {
    if (!tty) return 0;
    tty->error = 0;
    ssize_t len = read(tty->fd, data, size);
    if (len < 0) {
        tty->error = errno;
        return 0;
    }
    return len;
}

size_t tty_write(tty_ctx *tty, const void *data, size_t size) {
    if (!tty) return 0;
    tty->error = 0;
    ssize_t len = write(tty->fd, data, size);
    if (len < 0) {
        tty->error = errno;
        return 0;
    }
    return len;
}

bool tty_wait(tty_ctx *tty, int timeout) {
    if (!tty) return false;
    tty->error = 0;
    struct pollfd pfd;
    pfd.fd = tty->fd;
    pfd.revents = 0;
    pfd.events = POLLIN;
    int status = poll(&pfd, 1, timeout);
    if (status < 0) {
        tty->error = errno;
        return false;
    }
    if (!status) {
        tty->error = ETIMEDOUT;
        return false;
    }
    if (pfd.revents & POLLHUP) { // NOLINT
        tty->error = ECONNRESET;
        return false;
    }
    return pfd.revents & POLLIN; // NOLINT
}

int tty_pending(tty_ctx *tty) {
    if (!tty) return 0;
    int count = 0;
    tty->error = 0;
    if (ioctl(tty->fd, FIONREAD, &count) == -1) {
        tty->error = errno;
        return 0;
    }
    return count;
}

__attribute__((format(printf, 3, 4))) bool tty_printf(tty_ctx *tty, size_t size, const char *fmt, ...) {
    if (!fmt) return false;
    char buf[size];
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    if (len < 0) {
        tty->error = errno;
        return false;
    }
    if (!len) return true;
    if (len >= size) {
        tty->error = EOVERFLOW;
        return false;
    }
    return tty_write(tty, buf, len) == len;
}

__attribute__((format(scanf, 3, 4))) int tty_scanf(tty_ctx *tty, size_t size, const char *fmt, ...) {
    if (!fmt) return EOF;
    char buf[size + 1];
    size_t len = tty_read(tty, buf, sizeof(buf));
    if (len <= 0) return EOF;
    buf[len] = 0;
    va_list args;
    va_start(args, fmt);
    int got = vsscanf(buf, fmt, args);
    va_end(args);
    return got;
}

size_t tty_gets(tty_ctx *tty, char *buf, size_t size) {
    if (!tty || !buf) return 0;
    buf[0] = 0;
    size_t len = tty_read(tty, buf, size);
    if (len < 1) return 0;
    if (len < size) buf[len] = 0;
    return size;
}

size_t tty_puts(tty_ctx *tty, const char *str, size_t size) {
    if (!tty || !str) return 0;
    size_t len = cpr_strlen(str, size);
    if (!len) return 0;
    return tty_write(tty, str, len);
}

int tty_ignore(tty_ctx *tty, int timeout) {
    if (!tty) return 0;
    int count = 0;
    while (tty_wait(tty, timeout)) {
        char buf;
        if (tty_read(tty, &buf, 1) < 1) return count;
        ++count;
    }
    return count;
}

__attribute__((noreturn)) __attribute__((format(printf, 3, 4))) void tty_crit(tty_ctx *tty, int exit_code, const char *fmt, ...) {
    tty_close(tty);
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    fputc('\n', stderr);
    fflush(stderr);
    va_end(args);
    quick_exit(exit_code);
}
#endif
