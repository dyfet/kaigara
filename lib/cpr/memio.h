// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#ifndef CPR_MEMIO_H
#define CPR_MEMIO_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char *data;
    size_t get, put, size;
    bool alloc;
    char buf[0]; // if allocated...
} memio_t;

void cpr_initmem(memio_t *mem, char *from, size_t size);
void cpr_freemem(memio_t *mem);
void cpr_resetmem(memio_t *mem);
char cpr_cgetmem(memio_t *mem);
bool cpr_cputmem(memio_t *mem, char ch);
bool cpr_sputmem(memio_t *mem, const char *text);
bool cpr_xputmem(memio_t *mem, const char *from, size_t size);
bool cpr_fmtmem(memio_t *mem, size_t estimated, const char *fmt, ...);
const char *cpr_lgetmem(memio_t *mem, size_t *out, const char *delim);
const void *cpr_xgetmem(memio_t *mem, size_t size);
memio_t *cpr_makemem(size_t size);

#ifdef __cplusplus
}
#endif
#endif
