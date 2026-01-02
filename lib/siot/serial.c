// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#include "serial.h"
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32

#else

static bool sio_setup(sio_ctx *sio, char echo) {
    if (!sio) return false;
    long ioflags = fcntl(sio->fd, F_GETFL);
    tcgetattr(sio->fd, &sio->active);
    tcgetattr(sio->fd, &sio->saved);
    sio_config(sio);
    fcntl(sio->fd, F_SETFL, ioflags & ~O_NDELAY);
    sio->echo = echo;
    sio->error = 0;

#if defined(TIOCM_RTS) && defined(TIOCMODG)
    int mcs = 0;
    ioctl(sio->fd, TIOCMODG, &mcs);
    mcs |= TIOCM_RTS;
    ioctl(sio->fd, TIOCMODS, &mcs);
#endif
    return true;
}

bool sio_config(sio_ctx *sio) {
    if (!sio || !sio->fd) return false;
    sio->active.c_oflag = sio->active.c_lflag = 0;
    sio->active.c_cflag = CLOCAL | CREAD | HUPCL;
    sio->active.c_iflag = IGNBRK;

    memset(&sio->active.c_cc, 0, sizeof(sio->active.c_cc));
    sio->active.c_cc[VMIN] = 1;

    sio->active.c_cflag |= sio->saved.c_cflag & (CRTSCTS | CSIZE | PARENB | PARODD | CSTOPB);
    sio->active.c_iflag |= sio->saved.c_iflag & (IXON | IXANY | IXOFF);
    return tcsetattr(sio->fd, TCSANOW, &sio->active) == 0;
}

bool sio_open(sio_ctx *sio, const char *path, char echo) {
    if (!sio) return false;
    sio->fd = sio->out = open(path, O_RDWR | O_NDELAY); // FlawFinder: ignore
    if (sio->fd < 0) return false;
    return sio_setup(sio, echo);
}

bool sio_attach(sio_ctx *sio, char echo) {
    if (!sio || !isatty(0)) return false;
    sio->fd = 0;
    if (isatty(1))
        sio->out = 1;
    else
        sio->out = -1;
    return sio_setup(sio, echo);
}

bool sio_close(sio_ctx *sio) {
    if (!sio) return false;
    if (sio->fd < 0) return false;
    tcsetattr(sio->fd, TCSANOW, &sio->saved);
    if (sio->fd > 2)
        close(sio->fd);
    return true;
}

int sio_canon(sio_ctx *sio, const char *nl) {
    char nl1 = 0, nl2 = 0;

    if (!sio || !sio->fd)
        return 0;

    if (!nl)
        return 0;

    if (*nl) {
        nl1 = nl[0];
        nl2 = nl[1];
    }

    struct termios *attr = &sio->active;
    attr->c_cc[VMIN] = nl1 ? 0 : 1;
    attr->c_cc[VTIME] = 0;
    attr->c_cc[VEOL] = nl1;
    attr->c_cc[VEOL2] = nl2;
    attr->c_lflag |= ICANON;
    tcsetattr(sio->fd, TCSANOW, attr);

#ifdef _PC_MAX_CANON
    return (int)fpathconf(sio->fd, _PC_MAX_CANON);
#else
    return MAX_CANON
#endif
}

int sio_packet(sio_ctx *sio, size_t size, uint8_t timer) {
    if (!sio || !sio->fd)
        return 0;

#ifdef _PC_MAX_INPUT
    long max = fpathconf(sio->fd, _PC_MAX_INPUT);
#else
    long max = MAX_INPUT;
#endif

    if (size > (size_t)max)
        return 0;

    struct termios *attr = &sio->active;
    attr->c_cc[VEOL] = attr->c_cc[VEOL2] = 0;
    attr->c_cc[VMIN] = (uint8_t)size;
    attr->c_cc[VTIME] = timer;
    attr->c_lflag &= ~ICANON;
    tcsetattr(sio->fd, TCSANOW, attr);
    return (int)size;
}

bool sio_putch(sio_ctx *ctx, char ch) {
    if (!ctx) return false;
    ctx->error = 0;
    ssize_t rtn = write(ctx->out, &ch, 1);
    if (rtn < 0) ctx->error = errno;
    return rtn == 1;
}

char sio_getch(sio_ctx *ctx) {
    if (!ctx) return 0;
    ctx->error = 0;
    char ch;
    ssize_t rtn = read(ctx->fd, &ch, 1); // FlawFinder: valid
    if (rtn < 0) ctx->error = errno;
    if (rtn != 1) return 0;
    if (ctx->echo > 1 && ch > 31 && ch != 127) {
        if (!sio_putch(ctx, ctx->echo))
            return 0;
    } else if (ctx->echo) {
        if (!sio_putch(ctx, ch))
            return 0;
    }
    return ch;
}

ssize_t sio_putline(sio_ctx *ctx, const char *str) {
    if (!str || !ctx) return -1;
    ctx->error = 0;
    ssize_t rtn = write(ctx->out, str, strlen(str)); // FlawFinder: ignore
    if (rtn < 0) ctx->error = errno;
    return rtn;
}

ssize_t sio_getline(sio_ctx *ctx, char *buf, size_t max, char eol) {
    if (!buf || !ctx || max < 1) return -1;
    *buf = 0;
    --max;

    size_t count = 0;
    while (count < max) {
        char ch = sio_getch(ctx);
        if (ch == 0 || ch == eol) break;
        buf[count++] = ch;
    }
    return (ssize_t)count;
}
#endif
