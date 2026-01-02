// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#ifndef CPR_MEMPAGER_H
#define CPR_MEMPAGER_H

#include <memory.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "strchar.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _mempage {
    struct _mempage *next, *prev;
    size_t used;
} *mempage_t;

typedef struct _mempager {
    mempage_t head, tail, free;
    size_t size;
    unsigned limit, count;
} *mempager_t;

mempager_t pager_create(size_t pagesize, unsigned max);
void *pager_data(mempage_t page);
void pager_reset(mempager_t pager);
void pager_free(mempager_t pager);
mempage_t pager_request(mempager_t pager);
void *pager_alloc(mempager_t pager, size_t size);
char *pager_strdup(mempager_t pager, const char *str);

#define PAGER(mem, T) (T *)pager_alloc(mem, sizeof(T))
#define MAKE_PAGER(T, max) pager_create(sizeof(T), max)
#define FREE_PAGER(ptr) pager_free(ptr)

#ifdef __cplusplus
}
#endif
#endif
