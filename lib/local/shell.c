// SPDX-License-Identifier: BSD-3-Clause
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#include <cpr/strchar.h>
#include <cpr/system.h>
#include <stdarg.h>
#include "shell.h"

#ifdef _WIN32
bool dev_name(const char *from, char *buf, size_t size) {
    if (eq(from, "-")) return cpr_strcpy(buf, DEV_NULL, size) != NULL;
    if (strchr(from, '/')) return false;
    if (cpr_strcpy(buf, from, size) != NULL) return false;
    char *sep = strchr(buf, 'L');
    if (sep) {
        *(sep++) = 0;
        if (*sep) return false;
    }
    return true;
}
#else
bool dev_name(const char *from, char *buf, size_t size) {
    if (eq(from, "-")) return cpr_strcpy(buf, DEV_NULL, size) != NULL;
    if (eq_n(from, "/dev/", 5)) return cpr_strcpy(buf, from, size) != NULL;
    if (strchr(from, '/')) return false;
    if (cpr_strcpy(buf, "/dev/", size) == NULL) return false;
    return cpr_strcat(buf, from, size) != NULL;
}
#endif

bool fix_path(char *path, bool dir) {
    if (!path) return false;
    char *cp = path;

#ifndef _WIN32
    while (NULL != (cp = strchr(cp, '\\')))
        *(cp++) = '/';
#endif

    char *tail = "";
    while (tail) {
        tail = strrchr(path, '/');
        if (!tail || tail[1] != 0) break;
        if (dir) {
            if (!is_dir(path)) return false;
            return true;
        }
        *tail = 0;
    }
    return true;
}

bool flag_one_none(unsigned count, ...) {
    bool one = false;
    bool any = false;
    va_list args;

    va_start(args, count);
    while (count--) {
        bool opt = va_arg(args, int);
        if (opt && !any) {
            one = true;
        } else {
            if (opt) one = false;
        }
        if (opt) any = true;
    }
    va_end(args);
    return one || !any;
}

bool flag_one_only(unsigned count, ...) {
    bool one = false;
    bool any = false;
    va_list args;

    va_start(args, count);
    while (count--) {
        bool opt = va_arg(args, int);
        if (opt && !any) {
            one = true;
        } else {
            if (opt) one = false;
        }
        if (opt) any = true;
    }
    va_end(args);
    return one;
}
