// SPDX-License-Identifier: BSD-3-Clause
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#ifndef LOCAL_SCAN_H
#define LOCAL_SCAN_H

#include <stdint.h>

#define bad_size ((uint64_t)-1)
#define bad_value (0)
#define bad_perms (0U)

int scan_value(const char *s, int min_val, int max_val);
char scan_char(const char *s);
char scan_quote(const char *s);
char scan_newline(const char *s);
uint64_t scan_size(const char *s);
unsigned scan_perms(const char *s);

#endif
