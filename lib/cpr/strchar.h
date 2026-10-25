// SPDX-License-Identifier: BSD-3-Clause
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#ifndef CPR_STRCHAR_H
#define CPR_STRCHAR_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>

#ifdef __cplusplus
extern "C" {
#endif

char *cpr_strtok(char *s, const char *delim, char **saveptr);
size_t cpr_strlen(const char *s, size_t max);
char *cpr_strcpy(char *m, const char *s, size_t max);
char *cpr_strcat(char *m, const char *s, size_t max);
char *cpr_append(char *m, size_t max, ...);
char *cpr_strdup(const char *str, size_t max);
void cpr_strfree(char **str);
char *cpr_upper(const char *s, size_t max);
char *cpr_lower(const char *s, size_t max);
char *cpr_getenv(const char *s, size_t max);
bool cpr_equal(const char *s1, const char *s2);
bool cpr_equal_n(const char *s1, const char *s2, size_t size);
bool cpr_match(const char *s1, const char *s2);
int cpr_order(const char *s1, const char *s2);
size_t cpr_strhead(char *str, size_t max, size_t count);
size_t cpr_strtail(char *str, size_t max, size_t count);
char *cpr_reverse(char *str);
char *cpr_strtrim(char *str, const char *list, size_t max);
char *cpr_strchop(char *str, const char *list, size_t max);
char *cpr_strlong(long value, char *buf, size_t size, int base);
bool cpr_ascii(const char *cp, size_t limit);
bool is_empty(const char *str);
bool is_text(const char *str);
bool is_symbol(const char *src, const char *allow);
bool is_ascii(const char *cp);

__attribute__((format(printf, 2, 3))) bool cpr_asprintf(char **strp, const char *fmt, ...);

__attribute__((format(printf, 3, 4))) bool cpr_elid(char *buf, size_t size, const char *fmt, ...);

inline static bool eq(const char *s1, const char *s2) {
    return cpr_equal(s1, s2);
}

inline static bool eq_n(const char *s1, const char *s2, size_t size) {
    return cpr_equal_n(s1, s2, size);
}

inline static bool match(const char *s1, const char *s2) {
    return cpr_match(s1, s2);
}

inline static bool is_char(const char *ptr) {
    return ptr && ptr[0] && ptr[1] == 0;
}

inline static char to_char(const char *ptr) {
    return is_char(ptr) ? ptr[0] : 0;
}

static inline bool is_true(char ch) {
    ch = (char)(tolower(ch));
    return ch == 't' || ch == 'y' || ch == '1';
}

static inline bool is_false(char ch) {
    ch = (char)(tolower(ch));
    return ch == 'f' || ch == 'm' || ch == '0';
}

#ifdef __cplusplus
}
#endif
#endif
