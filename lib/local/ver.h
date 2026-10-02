// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 202y David Sugar <tychosoft@gmail.com>

#ifndef LOCAL_VER_H
#define LOCAL_VER_H

#include <stdint.h>
#include <stddef.h>

#define VERINFO_SIZE 16

typedef uint8_t ver_t[VERINFO_SIZE];

void ver_init(ver_t *ver);
int ver_get(ver_t *ver, const char *str);
int ver_put(char *str, size_t len, const ver_t *ver);
int ver_hex(char *str, size_t len, const ver_t *ver);
int ver_cmp(const ver_t *v1, const ver_t *v2);

#endif
