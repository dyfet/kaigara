// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#ifndef SIOT_SERIAL_H
#define SIOT_SERIAL_H

#ifndef _WIN32
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <termios.h>

#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32

#else
typedef struct {
    struct termios saved, active;
    int fd, out;
    int error;
    char echo;
} sio_ctx;
#endif

bool sio_open(sio_ctx *sio, const char *path, char echo);
bool sio_attach(sio_ctx *sio, char echo);
bool sio_config(sio_ctx *sio);
bool sio_close(sio_ctx *sio);
int sio_canon(sio_ctx *sio, const char *nl);
int sio_packet(sio_ctx *sio, size_t size, uint8_t timer);
bool sio_putch(sio_ctx *sio, char ch);
char sio_getch(sio_ctx *ctx);
ssize_t sio_putline(sio_ctx *ctx, const char *str);
ssize_t sio_getline(sio_ctx *ctx, char *buf, size_t max, char eol);

#ifdef __cplusplus
}
#endif
#endif

#endif
