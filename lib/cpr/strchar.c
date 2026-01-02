// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#include "strchar.h"
#include "memory.h"
#include <string.h>

size_t cpr_strlen(const char *s, size_t max) {
    size_t count = 0;

    if (!s)
        return 0;

    while (*s && (++count < max))
        ++s;
    return count;
}

char *cpr_strcpy(char *m, const char *s, size_t max) {
    size_t size = cpr_strlen(s, max);
    if (!m || !s)
        return NULL;

    if (size >= max)
        size = max;

    cpr_memcpy(m, max, s, size);
    if (size)
        m[--size] = 0;
    return m;
}

char *cpr_strcat(char *m, const char *s, size_t max) {
    if (!m || !s)
        return NULL;

    size_t offset = cpr_strlen(m, max);
    if (offset >= max)
        return NULL;

    return cpr_strcpy(m + offset, s, max - offset);
}

char *cpr_strdup(const char *str, size_t max) {
    if (!str)
        return NULL;

    size_t len = cpr_strlen(str, max);
    char *out = (char *)malloc(++len);
    if (!out)
        return NULL;

    if (--len)
        // FlawFinder: ignore
        memcpy(out, str, len); // NOLINT

    out[len] = 0;
    return out;
}

char *cpr_upper(const char *s, size_t max) {
    if (!s)
        return NULL;

    size_t size = cpr_strlen(s, max);
    char *t = malloc(++size);
    if (!t)
        return NULL;

    char *m = t;
    while (--size > 0)
        *(t++) = (char)toupper(*(s++));
    *t = 0;
    return m;
}

char *cpr_lower(const char *s, size_t max) {
    if (!s)
        return NULL;

    size_t size = cpr_strlen(s, max);
    char *t = malloc(++size);
    if (!t)
        return NULL;

    char *m = t;
    while (--size > 0)
        *(t++) = (char)tolower(*(s++));
    *t = 0;
    return m;
}

char *cpr_getenv(const char *s, size_t max) {
    return cpr_strdup(getenv(s), max); // FlawFinder: ignore
}

bool cpr_equal(const char *s1, const char *s2) {
    if (s1 == s2)
        return true;

    if (!s1 || !s2)
        return false;

    return strcmp(s1, s2) == 0;
}

bool cpr_match(const char *s1, const char *s2) {
    if (s1 == s2)
        return true;

    if (!s1 || !s2)
        return false;

    return strcasecmp(s1, s2) == 0;
}

int cpr_order(const char *s1, const char *s2) {
    if (s1 == s2)
        return 0;

    if (!s1)
        return -1;
    else if (!s2)
        return 1;

    return strcoll(s1, s2);
}

size_t cpr_strhead(char *str, size_t max, size_t count) {
    size_t len = cpr_strlen(str, max);

    if (!str)
        return 0;

    if (count >= len) {
        *str = 0;
        return 0;
    }

    const char *tmp = str + count;
    while (*tmp)
        *(str++) = *(tmp++);
    *str = 0;
    return len - count;
}

size_t cpr_strtail(char *str, size_t max, size_t count) {
    size_t len = cpr_strlen(str, max);

    if (!str)
        return 0;

    if (count >= max)
        return 0;

    if (count >= len) {
        *str = 0;
        return len;
    }

    str[count] = 0;
    return count;
}

char *cpr_reverse(char *str) {
    char *p1, *p2;

    if (!str || !*str)
        return str;

    // FlawFinder: ignore
    for (p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2) {
        *p1 ^= *p2; // NOLINT(bugprone-narrowing-conversions)
        *p2 ^= *p1; // NOLINT(bugprone-narrowing-conversions)
        *p1 ^= *p2; // NOLINT(bugprone-narrowing-conversions)
    }
    return str;
}

char *cpr_strlong(long v, char *p, size_t s) {
    if (s < 2)
        return NULL;

    const char *e = p + (--s);
    char *out = p;

    if (!v) {
        *(p++) = '0';
        *p = 0;
        return out;
    }

    if (v < 0) {
        *(p++) = '-';
        v = -v;
    }

    char *r = p;
    while (v > 0 && r < e) {
        *(r++) = (char)((v % 10) + '0');
        v /= 10;
    }
    if (r == e)
        return NULL;

    *r = 0;
    cpr_reverse(p);
    return out;
}

char *cpr_strtrim(char *str, const char *list, size_t max) {
    if (!str)
        return NULL;

    if (!list || !*list)
        return str;

    size_t len = cpr_strlen(str, max - 1);
    while (*str && len--) {
        if (strchr(list, *str) == NULL)
            break;
        ++str;
    }
    return str;
}

char *cpr_strchop(char *str, const char *list, size_t max) {
    if (!str)
        return NULL;

    if (!list || !*list)
        return str;

    char *end = str + cpr_strlen(str, max - 1);
    while (--end >= str) {
        if (strchr(list, *end) == NULL)
            break;
        *end = 0;
    }
    return str;
}

bool is_empty(const char *str) {
    if (!str || !*str)
        return true;

    return false;
}
