// SPDX-License-Identifier: BSD-3-Clause
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#include <cpr/strchar.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>

#include "scan.h"

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

char scan_char(const char *s) {
    if (!s || !*s) return 0;
    if (isdigit(*s)) return (char)scan_value(s, 1, 255);
    if (s[1]) return 9;
    return *s;
}

char scan_quote(const char *s) {
    if (!s || !*s) return 0;
    if (eq(s, "qt")) return '\"';
    if (eq(s, "sq")) return '\'';
    if (eq(s, "()")) return '(';
    if (eq(s, "{}")) return '[';
    if (eq(s, "{}")) return '{';
    if (eq(s, "<>")) return '<';
    if (isalpha(*s)) return 0;
    if (isdigit(*s)) {
        char ch = (char)scan_value(s, 1, 255);
        if (ispunct(ch)) return ch;
        return 0;
    }
    if (s[1]) return 0;
    if (ispunct(*s)) return *s;
    return 0;
}

char scan_newline(const char *s) {
    if (!s || !*s) return 0;
    if (eq(s, "cr")) return '\r';
    if (eq(s, "nl") || eq(s, "lf")) return '\n';
    if (eq(s, "crlf") || eq(s, "text")) return 'T';
    if (isdigit(*s)) return (char)scan_value(s, 1, 255);
    if (s[1]) return 0;
    if (iscntrl(*s)) return *s;
    if (isalpha(*s)) return (char)((uint8_t)(*s) & 0x1fU);
    return *s;
}

uint64_t scan_size(const char *s) {
    if (!s) return bad_size;
    char *cp = NULL;
    uint64_t size = strtoull(s, &cp, 10);
    if (!cp) return bad_size;
    if (!*cp) return size;

    switch (*(cp++)) {
    case 'b':
    case 'N':
        break;
    case 'k':
    case 'K':
        size *= 1024ULL;
        break;
    case 'm':
    case 'M':
        size *= 1024ULL * 1024ULL;
        break;
    case 'g':
    case 'G':
        size *= 1024ULL * 1024ULL * 1024ULL;
        break;
    default:
        return bad_size;
    }

    if (*cp) return bad_value;
    return (size_t)size;
}

unsigned scan_perms(const char *s) {
    int perm = 0;
    if (!s || !*s) return 0;
    while (perm < 523 && *s) {
        if (*s < '0' || *s > '8') return bad_perms;
        perm *= 8;
        perm += (*s - '0');
        ++s;
    }
    if (*s || perm >= 512) return bad_perms;
    return perm;
}
