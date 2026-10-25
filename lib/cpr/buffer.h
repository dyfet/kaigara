// SPDX-License-Identifier: BSD-3-Clause
// Copyright (C) 2026 David Sugar <tychosoft@gmail.com>

#ifndef CPR_BUFFER_H
#define CPR_BUFFER_H

#include "thread.h"
#include "memory.h"

struct cpr_buffer {
    struct cpr_buffer *next;
    void *item;
};

typedef struct {
    mtx_t lock;
    cnd_t cond;
    struct cpr_buffer *head, *tail;
    size_t count, limit;
    bool closed;
    cpr_free_t free;
} buffer_t;

void purge_buffers();
void close_buffer(buffer_t *buf);
bool init_buffer(buffer_t *buf, size_t size, cpr_free_t ff);
void free_buffer(buffer_t *buf);
void *get_buffer(buffer_t *buf);
bool put_buffer(buffer_t *buf, void *ptr);
bool pri_buffer(buffer_t *buf, void *ptr);
bool is_buffer(buffer_t *buf);
#endif
