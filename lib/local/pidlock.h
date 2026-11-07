// SPDX-License-Identifier: BSD-3-Clause
// Copyright (C) 2026 David Sugar <tychosoft@gmail.com>

#ifndef LOCAL_PIDLOCK_H
#define LOCAL_PIDLOCK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    bool locked;
    intptr_t file;
    char path[512];
} pidlock_t;

bool pidlock_acquire(pidlock_t *lock, const char *path, bool blocking);
bool pidlock_release(pidlock_t *lock);
bool pidlock_update(pidlock_t *lock, const char *status);
long pidlock_process(const char *path);
void pidlock_cleanup(pidlock_t *lock);
void pidlock_close(pidlock_t *lock); // used in supervised forks

#ifdef __cplusplus
}
#endif
#endif
