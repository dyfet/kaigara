// SPDX-License-Identifier: BSD-3-Clause
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#ifndef CPR_SYNC_H
#define CPR_SYNC_H

#include "system.h"
#include "thread.h"
#include <time.h>
#include <stdbool.h>
#include <sys/time.h>

typedef struct timespec deadline_t;

void cpr_yield();
bool cpr_until(const deadline_t *ts);
void cpr_adjust(deadline_t *ts, long ms);
bool cpr_deadline(deadline_t *ts, long ms);
long cpr_expires(const deadline_t *deadline, struct timeval *tv);
long cpr_elapsed(const deadline_t *deadline);
bool cpr_realtime(const deadline_t *deadline, struct timespec *ts);

#endif
