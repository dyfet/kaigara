// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#include "thread.h"

void cor_condlock_init(cpr_condlock_t *lock) {
    if (!lock) return;
    lock->pending = lock->waiting = lock->sharing = 0;
    mtx_init(&lock->mtx, mtx_plain);
    cnd_init(&lock->bcast);
}

void cor_condlock_free(cpr_condlock_t *lock) {
    if (!lock) return;
    cnd_destroy(&lock->bcast);
    mtx_destroy(&lock->mtx);
}

void cor_condlock_access(cpr_condlock_t *lock) {
    if (!lock) return;
    mtx_lock(&lock->mtx);
    while (lock->pending) {
        ++lock->waiting;
        cnd_wait(&lock->bcast, &lock->mtx);
        --lock->waiting;
    }
    ++lock->sharing;
    mtx_unlock(&lock->mtx);
}

void cpr_condlock_release(cpr_condlock_t *lock) {
    if (!lock) return;
    mtx_lock(&lock->mtx);
    if (lock->sharing) {
        --lock->sharing;
        if (lock->pending && !lock->sharing)
            cnd_signal(&lock->bcast);
        else if (lock->waiting && !lock->pending)
            cnd_broadcast(&lock->bcast);
    }
    mtx_unlock(&lock->mtx);
}

void cpr_condlock_modify(cpr_condlock_t *lock) {
    if (!lock) return;
    mtx_lock(&lock->mtx);
    while (lock->sharing) {
        ++lock->pending;
        cnd_wait(&lock->bcast, &lock->mtx);
        --lock->pending;
    }
}

void cpr_condlock_commit(cpr_condlock_t *lock) {
    if (!lock) return;
    if (lock->pending)
        cnd_signal(&lock->bcast);
    else if (lock->waiting)
        cnd_broadcast(&lock->bcast);
    mtx_unlock(&lock->mtx);
}

void cpr_semaphore_init(cpr_semaphore_t *sem, unsigned limit) {
    if (!sem) return;
    sem->shutdown = false;
    sem->count = limit;
    sem->waits = sem->used = 0;
    mtx_init(&sem->mtx, mtx_plain);
    cnd_init(&sem->cond);
}

void cpr_semaphore_free(cpr_semaphore_t *sem) {
    if (!sem) return;
    cnd_destroy(&sem->cond);
    mtx_destroy(&sem->mtx);
}

bool cpr_semaphore_acquire(cpr_semaphore_t *sem) {
    if (!sem) return false;
    mtx_lock(&sem->mtx);
    while (sem->used >= sem->count && !sem->shutdown) {
        ++sem->waits;
        cnd_wait(&sem->cond, &sem->mtx);
        --sem->waits;
    }
    if (!sem->shutdown)
        ++sem->used;
    bool result = !sem->shutdown;
    mtx_unlock(&sem->mtx);
    return result;
}

void cpr_semaphore_shutdown(cpr_semaphore_t *sem) {
    if (!sem) return;
    mtx_lock(&sem->mtx);
    sem->shutdown = true;
    if (sem->waits)
        cnd_broadcast(&sem->cond);
    mtx_unlock(&sem->mtx);
}

void cpr_semaphore_release(cpr_semaphore_t *sem) {
    if (!sem) return;
    mtx_lock(&sem->mtx);
    if (sem->used) --sem->used;
    if (sem->waits) cnd_signal(&sem->cond);
    mtx_unlock(&sem->mtx);
}

void cpr_waitgroup_init(cpr_waitgroup_t *wg, unsigned count) {
    if (!wg) return;
    wg->count = count;
    mtx_init(&wg->mtx, mtx_plain);
    cnd_init(&wg->bcast);
}

void cpr_waitgroup_free(cpr_waitgroup_t *wg) {
    if (!wg) return;
    cnd_destroy(&wg->bcast);
    mtx_destroy(&wg->mtx);
}

void cpr_waitgroup_add(cpr_waitgroup_t *wg, unsigned count) {
    if (!wg) return;
    mtx_lock(&wg->mtx);
    wg->count += count;
    mtx_unlock(&wg->mtx);
}

void cpr_waitgroup_wait(cpr_waitgroup_t *wg) {
    if (!wg) return;
    mtx_lock(&wg->mtx);
    while (wg->count) {
        cnd_wait(&wg->bcast, &wg->mtx);
    }
    mtx_unlock(&wg->mtx);
}

void cpr_waitgroup_done(cpr_waitgroup_t *wg) {
    if (!wg) return;
    mtx_lock(&wg->mtx);
    if (wg->count) --wg->count;
    if (!wg->count)
        cnd_broadcast(&wg->bcast);
    mtx_unlock(&wg->mtx);
}

void cpr_waitgroup_finish(cpr_waitgroup_t *wg) {
    cpr_waitgroup_wait(wg);
    cpr_waitgroup_free(wg);
}
