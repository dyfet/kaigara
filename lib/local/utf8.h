// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 202y David Sugar <tychosoft@gmail.com>

#ifndef LOCAL_UTF8_H
#define LOCAL_UTF8_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define UTF8_MULTIPLIER 4
#ifndef UTF8_HEAP
#define UTF8_HEAP 1
#endif

typedef char *utf8_t;
typedef int32_t ucs4_t;
typedef ucs4_t *ucs4_p;

bool is_asc7(utf8_t from);
bool is_utf8(utf8_t from);
bool is_asc8(utf8_t from);
bool utf8_limit(utf8_t from, size_t limit);
size_t utf8_prefix(char code);
size_t utf8_size(utf8_t from);
ucs4_t utf8_code(utf8_t from);
size_t utf8_count(utf8_t from);
utf8_t utf8_index(utf8_t from, size_t index);
size_t utf8_chars(ucs4_t code, utf8_t to);
size_t utf8_expand(utf8_t from, ucs4_p to, size_t size);
size_t utf8_encode(utf8_t to, ucs4_p from, size_t size);

#if UTF8_HEAP > 0
utf8_t utf8_copy(utf8_t from, size_t codes);
void utf8_free(utf8_t str);
#endif

#ifdef __cplusplus
}
#endif
#endif
