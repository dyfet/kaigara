// SPDX-License-Identifier: GPL-3.0-or-later
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

size_t cpr_strlen(const char *s, size_t max);
char *cpr_strcpy(char *m, const char *s, size_t max);
char *cpr_strcat(char *m, const char *s, size_t max);
char *cpr_strdup(const char *str, size_t max);
char *cpr_upper(const char *s, size_t max);
char *cpr_lower(const char *s, size_t max);
char *cpr_getenv(const char *s, size_t max);
bool cpr_equal(const char *s1, const char *s2);
bool cpr_match(const char *s1, const char *s2);
int cpr_order(const char *s1, const char *s2);
size_t cpr_strhead(char *str, size_t max, size_t count);
size_t cpr_strtail(char *str, size_t max, size_t count);
char *cpr_reverse(char *str);
char *cpr_strlong(long v, char *p, size_t s);
char *cpr_strtrim(char *str, const char *list, size_t max);
char *cpr_strchop(char *str, const char *list, size_t max);
bool is_empty(const char *str);

inline static bool eq(const char *s1, const char *s2) {
    return cpr_equal(s1, s2);
}

inline static bool match(const char *s1, const char *s2) {
    return cpr_match(s1, s2);
}

#ifdef __cplusplus
}
#endif
#endif
