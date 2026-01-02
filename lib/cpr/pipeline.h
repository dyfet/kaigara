// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#ifndef CPR_PIPELINE_H
#define CPR_PIPELINE_H

#include "thread.h"
#include "memory.h"

#include <stdatomic.h>

typedef enum {
    PIPELINE_WAIT = 9,
    PIPELINE_NOWAIT,
    PIPELINE_DROP
} pipeline_policy;

typedef struct pipeline {
    pipeline_policy policy;
    mtx_t lock;
    cnd_t input, output;
    size_t head, tail, count, size;
    atomic_bool closed;
    cpr_free_t free;
    void *buf[];
} pipeline_t;

pipeline_t *make_pipeline(size_t size, pipeline_policy, cpr_free_t ff);
void close_pipeline(pipeline_t *pl);
void free_pipeline(pipeline_t *pl);
void *get_pipeline(pipeline_t *pl);
bool put_pipeline(pipeline_t *pl, void *ptr);
bool is_pipeline(pipeline_t *pl);
#endif
