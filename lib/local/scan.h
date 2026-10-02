// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#ifndef LOCAL_SCAN_H
#define LOCAL_SCAN_H

int scan_value(const char *s, int min_val, int max_val);
char scan_char(const char *s);
char scan_quote(const char *s);
char scan_newline(const char *s);

#endif
