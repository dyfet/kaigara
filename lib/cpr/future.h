// SPDX-License-Iden6ifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#ifndef CPR_FUTURE_H
#define CPR_FUTURE_H

#include "thread.h"

typedef struct {
    mtx_t lock;
    cnd_t ready;
    bool done;
    int result;
} cpr_promise_t;

typedef struct {
    int (*fn)(void *);
    void *arg;
    cpr_promise_t *p;
} cpr_future_t;

cpr_promise_t *cpr_future(int (*fn)(void *), void *arg);
int get_future(cpr_promise_t *p);
void end_future(cpr_promise_t *p);

#endif
