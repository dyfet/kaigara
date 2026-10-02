// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 David Sugar <tychosoft@gmail.com>

#ifndef LOCAL_XDG_H
#define LOCAL_XDG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>

typedef struct {
    char *desktop;
    char *documents;
    char *downloads;
    char *music;
    char *videos;
    char *pictures;
    char *public;
    char *temp;
    char *config;
    char *cache;
    char *data;
    char *state;
} xdg_t;

char *xdg_desktop();
char *xdg_userdir();
bool xdg_init(xdg_t *xdg);
void xdg_free(xdg_t *xdg);

#ifdef __cplusplus
}
#endif
#endif
