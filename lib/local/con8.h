// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 202y David Sugar <tychosoft@gmail.com>

#ifndef LOCAL_CON8_H
#define LOCAL_CON8_H

#ifdef __cplusplus
extern "C" {
#endif

#include "utf8.h"
#include "con.h"

bool con8_utf8();
bool con8_putc(ucs4_t code);
size_t con8_puts(utf8_t from, size_t cpmax);
size_t con8_gets(int timeout, utf8_t out);
ucs4_t con8_inkey(int timeout);

#ifdef __cplusplus
}
#endif
#endif
