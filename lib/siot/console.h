// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#ifndef SIOT_CONSOLE_H
#define SIOT_CONSOLE_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <signal.h>

#ifdef __cplusplus
extern "C" {
#endif

bool sio_startup();
bool sio_wrap(bool enable);
void sio_cls();
void sio_reset();
bool sio_passwd(char *buf, size_t size, const char *prompt);

#ifdef __cplusplus
}
#endif
#endif
