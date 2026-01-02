// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#include <errno.h>
#include <limits.h>
#include <stdlib.h>

int scan_value(const char *s, int min_val, int max_val) {
    if (!s || !*s) return 0;

    errno = 0;
    char *end = NULL;
    long v = strtol(s, &end, 10);
    if (end == s) return 0;
    if (*end != '\0') return 0;
    if (errno == ERANGE) return 0;
    if (v < min_val || v > max_val) return 0;
    return (int)v;
}
