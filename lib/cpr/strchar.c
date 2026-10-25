// SPDX-License-Identifier: BSD-3-Clause
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#include "strchar.h"
#include "memory.h"
#include <string.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdarg.h>

char *cpr_strtok(char *s, const char *delim, char **saveptr) {
    if (s != NULL) *saveptr = s;
    if (*saveptr == NULL) return NULL;

    char *start = *saveptr;
    start += strspn(start, delim);
    if (*start == '\0') {
        *saveptr = NULL;
        return NULL;
    }

    char *end = start + strcspn(start, delim);
    if (*end != '\0') {
        *end = '\0';
        *saveptr = end + 1;
    } else {
        *saveptr = NULL;
    }
    return start;
}

size_t cpr_strlen(const char *s, size_t max) {
    size_t count = 0;
    if (!s) return 0;
    while (*s && (++count < max))
        ++s;
    return count;
}

char *cpr_strcpy(char *m, const char *s, size_t max) {
    size_t size = cpr_strlen(s, max);
    if (!m || !s) return NULL;
    if (size >= max) size = max - 1;
    if (size) cpr_memcpy(m, max, s, size);
    m[size] = 0;
    return m;
}

char *cpr_strcat(char *m, const char *s, size_t max) {
    if (!m || !s) return NULL;
    size_t offset = cpr_strlen(m, max);
    if (offset >= max) return NULL;
    return cpr_strcpy(m + offset, s, max - offset);
}

char *cpr_strdup(const char *str, size_t max) {
    if (!str) return NULL;
    size_t len = cpr_strlen(str, max);
    char *out = (char *)malloc(++len);
    if (!out) return NULL;
    if (--len) cpr_memcpy(out, len, str, len);
    out[len] = 0;
    return out;
}

void cpr_strfree(char **str) {
    if (str && *str) free(*str);
    if (str) *str = NULL;
}

char *cpr_append(char *m, size_t max, ...) {
    if (!m) return NULL;
    va_list list;
    va_start(list, max);
    for (;;) {
        const char *ap = va_arg(list, const char *); // NOLINT
        if (!ap) break;
        cpr_strcat(m, ap, max);
    }
    va_end(list);
    return m;
}

char *cpr_upper(const char *s, size_t max) {
    if (!s) return NULL;
    size_t size = cpr_strlen(s, max);
    char *t = malloc(++size);
    if (!t) return NULL;
    char *m = t;
    while (--size > 0)
        *(t++) = (char)toupper(*(s++));
    *t = 0;
    return m;
}

char *cpr_lower(const char *s, size_t max) {
    if (!s) return NULL;
    size_t size = cpr_strlen(s, max);
    char *t = malloc(++size);
    if (!t) return NULL;

    char *m = t;
    while (--size > 0)
        *(t++) = (char)tolower(*(s++));
    *t = 0;
    return m;
}

char *cpr_getenv(const char *s, size_t max) {
    return cpr_strdup(getenv(s), max);
}

bool cpr_equal(const char *s1, const char *s2) {
    if (s1 == s2) return true;
    if (!s1 || !s2) return false;
    return strcmp(s1, s2) == 0;
}

bool cpr_equal_n(const char *s1, const char *s2, size_t size) {
    if (s1 == s2) return true;
    if (!s1 || !s2) return false;
    if (!size) return true;
    return strncmp(s1, s2, size) == 0;
}

bool cpr_match(const char *s1, const char *s2) {
    if (s1 == s2) return true;
    if (!s1 || !s2) return false;
    return strcasecmp(s1, s2) == 0;
}

int cpr_order(const char *s1, const char *s2) {
    if (s1 == s2) return 0;
    if (!s1)
        return -1;
    else if (!s2)
        return 1;

    return strcoll(s1, s2);
}

size_t cpr_strhead(char *str, size_t max, size_t count) {
    size_t len = cpr_strlen(str, max);
    if (!str) return 0;
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
    if (!str) return 0;
    if (count >= max) return 0;
    if (count >= len) {
        *str = 0;
        return len;
    }

    str[count] = 0;
    return count;
}

char *cpr_reverse(char *str) {
    uint8_t *p1, *p2;
    if (!str || !*str) return str;

    for (p1 = (uint8_t *)str, p2 = (uint8_t *)(str + strlen(str) - 1); p2 > p1; ++p1, --p2) {
        *p1 ^= *p2; // NOLINT(bugprone-narrowing-conversions)
        *p2 ^= *p1; // NOLINT(bugprone-narrowing-conversions)
        *p1 ^= *p2; // NOLINT(bugprone-narrowing-conversions)
    }
    return str;
}

char *cpr_strtrim(char *str, const char *list, size_t max) {
    if (!str) return NULL;
    if (!list || !*list) return str;
    size_t len = cpr_strlen(str, max - 1);
    while (*str && len--) {
        if (strchr(list, *str) == NULL) break;
        ++str;
    }
    return str;
}

char *cpr_strchop(char *str, const char *list, size_t max) {
    if (!str) return NULL;
    if (!list || !*list) return str;
    char *end = str + cpr_strlen(str, max - 1);
    while (--end >= str) {
        if (strchr(list, *end) == NULL) break;
        *end = 0;
    }
    return str;
}

bool is_empty(const char *str) {
    if (!str || !*str) return true;
    return false;
}

bool is_text(const char *cp) {
    if (!cp) return false;
    while (*cp) {
        if ((*cp > 0 && *cp < 32) || (*cp == 127)) return false;
        ++cp;
    }
    return true;
}

bool is_symbol(const char *cp, const char *allow) {
    if (!cp) return false;
    while (*cp) {
        if (allow && strchr(allow, *cp)) goto allowed;
        if ((*cp > 0 && *cp <= 32) || (*cp == 127)) return false;
        if (ispunct(*cp)) return false;
        if (iscntrl(*cp)) return false;
    allowed:
        ++cp;
    }
    return true;
}

bool is_ascii(const char *cp) {
    if (!cp) return false;
    while (*cp) {
        unsigned char ch = (unsigned char)*cp;
        if (ch >= 127) return false;
        ++cp;
    }
    return true;
}

bool cpr_ascii(const char *cp, size_t limit) {
    if (!cp) return false;
    size_t total = 0;
    while (*cp && total < limit) {
        unsigned char ch = (unsigned char)*cp;
        if (ch > 127) return false;
        ++total;
        ++cp;
    }
    return (!*cp) && (total <= limit);
}

char *cpr_strlong(long value, char *buf, size_t size, int base) {
    static const char digits[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    if (size == 0 || base < 2 || base > 36) return NULL;

    size_t i = size - 1;
    unsigned long v;
    int negative = 0;
    buf[i] = 0;
    if (value < 0) {
        negative = 1;
        v = (unsigned long)(-value);
    } else {
        v = (unsigned long)value;
    }

    do {
        if (i == 0) return NULL;
        buf[--i] = digits[v % base];
        v /= base;
    } while (v > 0);

    if (negative) {
        if (i == 0) return NULL;
        buf[--i] = '-';
    }

    size_t start = i;
    size_t j = 0;
    while (buf[start] != '\0') {
        buf[j++] = buf[start++];
    }
    buf[j] = 0;
    return buf;
}

__attribute__((format(printf, 3, 4))) bool cpr_elid(char *buf, size_t size, const char *fmt, ...) {
    if (!buf || size < 3 || !fmt) return false;
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buf, size, fmt, args);
    va_end(args);
    if (len < 0) return false;
    if (len >= size) {
        buf[len] = 0;
        buf[len - 1] = '.';
        buf[len - 2] = '.';
    }
    return true;
}

__attribute__((format(printf, 2, 3))) bool cpr_asprintf(char **strp, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    va_list ap_copy;
    va_copy(ap_copy, ap);
    int len = vsnprintf(NULL, 0, fmt, ap_copy);
    va_end(ap_copy);

    if (len < 0) {
        va_end(ap);
        return false;
    }

    char *buf = malloc(len + 1);
    if (!buf) {
        va_end(ap);
        return false;
    }

    int len2 = vsnprintf(buf, len + 1, fmt, ap);
    va_end(ap);
    if (len2 < 0) {
        free(buf);
        return false;
    }

    *strp = buf;
    return true;
}
