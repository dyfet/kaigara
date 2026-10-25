// SPDX-License-Identifier: BSD-3-Clause
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#include "sync.h"

#ifdef _WIN32
void cpr_yield() {
    Sleep(0);
    pthread_testcancel();
}

bool cpr_deadline(deadline_t *ts, long ms) {
    if (!ts) return false;
    LARGE_INTEGER freq, counter;
    if (!QueryPerformanceFrequency(&freq) || !QueryPerformanceCounter(&counter))
        return false;

    const long long ticks = counter.QuadPart;
    const long long sec = ticks / freq.QuadPart;
    const long long rem = ticks % freq.QuadPart;
    long long nsec = (rem * 1000000000LL) / freq.QuadPart;

    nsec = ((nsec + 5000LL) / 10000LL) * 10000LL;
    ts->tv_sec = (time_t)(sec);
    ts->tv_nsec = (long)(nsec);
    ts->tv_nsec += (ms % 1000) * 1000000L;
    ts->tv_sec += ms / 1000;
    if (ts->tv_nsec >= 1000000000L) {
        ts->tv_sec += 1;
        ts->tv_nsec -= 1000000000L;
    }
    cpr_adjust(ts, ms);
    pthread_testcancel();
    return true;
}

bool cpr_until(const deadline_t *ts) {
    long msecs = cpr_expires(ts, NULL);
    if (msecs <= 0) return true;
    Sleep((DWORD)(msecs));
    pthread_testcancel();
    return true;
}

#else

void cpr_yield() { sched_yield(); }

bool cpr_deadline(deadline_t *ts, long ms) {
    if (!ts) return false;
    if (clock_gettime(CLOCK_MONOTONIC, ts) != 0) return false;
    cpr_adjust(ts, ms);
    return true;
}

bool cpr_until(const deadline_t *ts) {
    return clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, ts, NULL) == 0;
}

#endif

void cpr_adjust(deadline_t *ts, long ms) {
    if (!ts) return;
    ts->tv_sec += ms / 1000;
    ts->tv_nsec += (ms % 1000) * 1000000;
    if (ts->tv_nsec >= 1000000000) {
        ts->tv_sec += 1;
        ts->tv_nsec -= 1000000000;
    } else if (ts->tv_nsec < 0) {
        ts->tv_sec -= 1;
        ts->tv_nsec += 1000000000L;
    }
}

long cpr_elapsed(const deadline_t *deadline) {
    struct timespec current, delta;
    if (!deadline) return 0;
    if (!cpr_deadline(&current, 0)) return 0;
    delta.tv_sec = current.tv_sec - deadline->tv_sec;
    delta.tv_nsec = current.tv_nsec - deadline->tv_nsec;
    if (delta.tv_nsec < 0) {
        delta.tv_sec -= 1;
        delta.tv_nsec += 1000000000;
    }
    if (delta.tv_sec < 0 || (delta.tv_sec == 0 && delta.tv_nsec <= 0)) return 0;
    return (delta.tv_sec * 1000) + (delta.tv_nsec / 1000000);
}

long cpr_expires(const deadline_t *deadline, struct timeval *tv) {
    struct timespec current, delta;
    if (tv) {
        tv->tv_sec = 0;
        tv->tv_usec = 0;
    }

    if (!deadline) return 0;
    if (!cpr_deadline(&current, 0)) return 0;
    delta.tv_sec = deadline->tv_sec - current.tv_sec;
    delta.tv_nsec = deadline->tv_nsec - current.tv_nsec;
    if (delta.tv_nsec < 0) {
        delta.tv_sec -= 1;
        delta.tv_nsec += 1000000000;
    }

    if (delta.tv_sec < 0 || (delta.tv_sec == 0 && delta.tv_nsec <= 0)) return 0;
    if (tv) {
        tv->tv_sec = delta.tv_sec;
        tv->tv_usec = delta.tv_nsec / 1000;
    }
    return (delta.tv_sec * 1000) + (delta.tv_nsec / 1000000);
}

bool cpr_realtime(const deadline_t *deadline, struct timespec *ts) {
    clock_gettime(CLOCK_REALTIME, ts);
    long ms = cpr_expires(deadline, NULL);
    if (!ms) return false;
    cpr_adjust(ts, ms);
    return true;
}
