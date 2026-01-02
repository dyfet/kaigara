// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#ifndef CPR_KEYFILE_H
#define CPR_KEYFILE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "strchar.h"

typedef struct _keydata {
    struct _keydata *next;
    char *id;
    char *value;
} keydata_t;

typedef struct _keysection {
    struct _keysection *next;
    keydata_t *keys;
    char *id;
} keysection_t;

keysection_t *append_keysections(keysection_t *root, keysection_t *into);
keysection_t *make_keysection(const char *id, keysection_t *prior);
keysection_t *find_keysection(keysection_t *root, const char *id);
keysection_t *load_keyfile(const char *path);
keydata_t *make_keydata(keysection_t *group, const char *id, const char *value);
const char *get_keyvalue(keysection_t *section, const char *id);
bool save_keyfile(const char *path, keysection_t *root);
void free_keufile(keysection_t *root);

#ifdef __cplusplus
}
#endif
#endif
