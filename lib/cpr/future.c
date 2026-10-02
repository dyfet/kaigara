// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 David Sugar <tychosoft@gmail.com>

#include "future.h"

static int future_thread(void *vp) {
    cpr_future_t *t = vp;
    int r = t->fn(t->arg);
    mtx_lock(&t->p->lock);
    t->p->result = r;
    t->p->done = true;
    cnd_signal(&t->p->ready);
    mtx_unlock(&t->p->lock);
    free(t);
    return 0;
}

cpr_promise_t *cpr_future(int (*fn)(void *), void *arg) {
    cpr_promise_t *p = malloc(sizeof(cpr_promise_t));
    if (!p) return NULL;

    mtx_init(&p->lock, mtx_plain);
    cnd_init(&p->ready);
    p->done = false;

    cpr_future_t *f = malloc(sizeof(cpr_future_t));
    if (!f) {
        end_future(p);
        return NULL;
    }

    f->fn = fn;
    f->arg = arg;
    f->p = p;
    thrd_t tid;
    if (thrd_create(&tid, future_thread, f) != thrd_success) {
        free(f);
        end_future(p);
        return NULL;
    }

    thrd_detach(tid);
    // ownership transferred f to future_thread(); freed there
    // cppcheck-suppress memleak
    // NOLINTNEXTLINE(misc-misplaced-alloc)
    return p;
}

int get_future(cpr_promise_t *p) {
    while (!p->done)
        cnd_wait(&p->ready, &p->lock);
    int r = p->result;
    mtx_unlock(&p->lock);
    return r;
}

void end_future(cpr_promise_t *p) {
    mtx_destroy(&p->lock);
    cnd_destroy(&p->ready);
    free(p);
}
