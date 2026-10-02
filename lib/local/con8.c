// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 202y David Sugar <tychosoft@gmail.com>

#include "con8.h"

#ifdef _WIN32
#define MICROSOFT_WINDOWS_WINBASE_H_DEFINE_INTERLOCKED_CPLUSPLUS_OVERLOADS 0
#include <windows.h>
#else
#define _XOPEN_SOURCE 700
#include <locale.h>
#include <langinfo.h>
#include <locale.h>
#endif

#include <string.h>
#include <stdio.h>

#ifdef _WIN32

bool con8_utf8() {
    if (!con_istty()) return false;
    return GetConsoleOutputCP() == 65001;
}

ucs4_t con8_inkey(int timeout) {
    return (ucs4_t)con_inkey(timeout);
}

size_t con8_gets(int timeout, utf8_t out) {
    if (!out) return 0;
    ucs4_t code = con8_inkey(timeout);
    if (code == EOF) return 0;
    return utf8_chars(code, out);
}

#else

bool con8_utf8() {
    if (!con_istty()) return false;
    setlocale(LC_CTYPE, "");
    return strcmp(nl_langinfo(CODESET), "UTF-8") == 0;
}

size_t con8_gets(int timeout, utf8_t out) {
    if (!out) return 0;
    unsigned char *encode = (unsigned char *)out;

    int code = con_inkey(timeout);
    if (code == EOF) return 0;
    encode[0] = (uint8_t)code;

    size_t add = utf8_size(out);
    size_t pos = 1;
    if (!add) return 0;
    while (pos < add) {
        code = con_inkey(KEY_INPUT_DEFAULT);
        if (code == EOF) return 0;
        encode[pos++] = (uint8_t)code;
    }
    return pos;
}

ucs4_t con8_inkey(int timeout) {
    char buf[4];
    if (con8_gets(timeout, buf) == 0) return false;
    return utf8_code(buf);
}

#endif

bool con8_putc(ucs4_t code) {
    char buf[UTF8_MULTIPLIER];
    size_t size = utf8_chars(code, buf);
    if (!size) return false;
    if (fwrite(buf, size, 1, stdout) != 1) return false;
    fflush(stdout);
    return true;
}

size_t con8_puts(utf8_t from, size_t cpmax) {
    if (!from) return 0;
    size_t size, count = 0;
    while (count < cpmax && *from && ferror(stdout) == 0) {
        size = utf8_size(from);
        if (!size) break;
        if (fwrite(from, size, 1, stdout) != 1) return 0;
        from += size;
        ++count;
    }
    fflush(stdout);
    return count;
}
