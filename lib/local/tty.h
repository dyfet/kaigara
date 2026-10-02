// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#if __has_include(<termios.h>) && !defined(_WIN32)
#ifndef LOCAL_TTY_H
#define LOCAL_TTY_H

#include <termios.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

typedef struct {
    struct termios saved, active;
    int fd, error;
    char nl1, nl2;
    uint8_t packet;
} tty_ctx;

bool tty_ispath(const char *path);
bool tty_open(tty_ctx *tty, const char *path);
bool tty_reset(tty_ctx *tty);
bool tty_close(tty_ctx *tty);
bool tty_softfc(tty_ctx *tty, bool enable);
bool tty_hardfc(tty_ctx *tty, bool enable);
bool tty_speed(tty_ctx *ctx, long speed);
bool tty_format(tty_ctx *tty, const char *format);
int tty_size(tty_ctx *tty);
int tty_line(tty_ctx *tty, const char *nl);
int tty_packet(tty_ctx *tty, uint8_t size, uint8_t timer);
size_t tty_read(tty_ctx *tty, void *data, size_t size);
size_t tty_write(tty_ctx *tty, const void *data, size_t size);
size_t tty_gets(tty_ctx *tty, char *buf, size_t size);
size_t tty_puts(tty_ctx *tty, const char *str, size_t size);
bool tty_echo(tty_ctx *tty, bool echo);
bool tty_shell(tty_ctx *tty, bool anable);
bool tty_sync(tty_ctx *tty);
bool tty_wait(tty_ctx *tty, int timeout);
int tty_pending(tty_ctx *tty);
int tty_ignore(tty_ctx *tty, int timeout);
FILE *tty_file(tty_ctx *tty, const char *mode);
__attribute__((format(printf, 3, 4))) bool tty_printf(tty_ctx *tty, size_t size, const char *fmt, ...);
__attribute__((format(scanf, 3, 4))) int tty_scanf(tty_ctx *tty, size_t size, const char *fmt, ...);
__attribute__((noreturn)) __attribute__((format(printf, 3, 4))) void tty_crit(tty_ctx *tty, int exit_code, const char *fmt, ...);
#endif
#endif
