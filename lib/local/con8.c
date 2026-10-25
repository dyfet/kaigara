// SPDX-License-Identifier: BSD-3-Clause
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

static char *pcmap[128] = {
/* 0x80 */ "Ç",
/* 0x81 */ "ü",
/* 0x82 */ "é",
/* 0x83 */ "â",
/* 0x84 */ "ä",
/* 0x85 */ "à",
/* 0x86 */ "å",
/* 0x87 */ "ç",
/* 0x88 */ "ê",
/* 0x89 */ "ë",
/* 0x8A */ "è",
/* 0x8B */ "ï",
/* 0x8C */ "î",
/* 0x8D */ "ì",
/* 0x8E */ "Ä",
/* 0x8F */ "Å",

/* 0x90 */ "É",
/* 0x91 */ "æ",
/* 0x92 */ "Æ",
/* 0x93 */ "ô",
/* 0x94 */ "ö",
/* 0x95 */ "ò",
/* 0x96 */ "û",
/* 0x97 */ "ù",
/* 0x98 */ "ÿ",
/* 0x99 */ "Ö",
/* 0x9A */ "Ü",
/* 0x9B */ "¢",
/* 0x9C */ "£",
/* 0x9D */ "¥",
/* 0x9E */ "₧",
/* 0x9F */ "ƒ",

/* 0xA0 */ "á",
/* 0xA1 */ "í",
/* 0xA2 */ "ó",
/* 0xA3 */ "ú",
/* 0xA4 */ "ñ",
/* 0xA5 */ "Ñ",
/* 0xA6 */ "ª",
/* 0xA7 */ "º",
/* 0xA8 */ "¿",
/* 0xA9 */ "⌐",
/* 0xAA */ "¬",
/* 0xAB */ "½",
/* 0xAC */ "¼",
/* 0xAD */ "¡",
/* 0xAE */ "«",
/* 0xAF */ "»",

/* 0xB0 */ "░",
/* 0xB1 */ "▒",
/* 0xB2 */ "▓",
/* 0xB3 */ "│",
/* 0xB4 */ "┤",
/* 0xB5 */ "╡",
/* 0xB6 */ "╢",
/* 0xB7 */ "╖",
/* 0xB8 */ "╕",
/* 0xB9 */ "╣",
/* 0xBA */ "║",
/* 0xBB */ "╗",
/* 0xBC */ "╝",
/* 0xBD */ "╜",
/* 0xBE */ "╛",
/* 0xBF */ "┐",

/* 0xC0 */ "└",
/* 0xC1 */ "┴",
/* 0xC2 */ "┬",
/* 0xC3 */ "├",
/* 0xC4 */ "─",
/* 0xC5 */ "┼",
/* 0xC6 */ "╞",
/* 0xC7 */ "╟",
/* 0xC8 */ "╚",
/* 0xC9 */ "╔",
/* 0xCA */ "╩",
/* 0xCB */ "╦",
/* 0xCC */ "╠",
/* 0xCD */ "═",
/* 0xCE */ "╬",
/* 0xCF */ "╧",

/* 0xD0 */ "╨",
/* 0xD1 */ "╤",
/* 0xD2 */ "╥",
/* 0xD3 */ "╙",
/* 0xD4 */ "╘",
/* 0xD5 */ "╒",
/* 0xD6 */ "╓",
/* 0xD7 */ "╫",
/* 0xD8 */ "╪",
/* 0xD9 */ "┘",
/* 0xDA */ "┌",
/* 0xDB */ "█",
/* 0xDC */ "▄",
/* 0xDD */ "▌",
/* 0xDE */ "▐",
/* 0xDF */ "▀",

/* 0xE0 */ "α",
/* 0xE1 */ "ß",
/* 0xE2 */ "Γ",
/* 0xE3 */ "π",
/* 0xE4 */ "Σ",
/* 0xE5 */ "σ",
/* 0xE6 */ "µ",
/* 0xE7 */ "τ",
/* 0xE8 */ "Φ",
/* 0xE9 */ "Θ",
/* 0xEA */ "Ω",
/* 0xEB */ "δ",
/* 0xEC */ "∞",
/* 0xED */ "φ",
/* 0xEE */ "ε",
/* 0xEF */ "∩",

/* 0xF0 */ "≡",
/* 0xF1 */ "±",
/* 0xF2 */ "≥",
/* 0xF3 */ "≤",
/* 0xF4 */ "⌠",
/* 0xF5 */ "⌡",
/* 0xF6 */ "÷",
/* 0xF7 */ "≈",
/* 0xF8 */ "°",
/* 0xF9 */ "∙",
/* 0xFA */ "·",
/* 0xFB */ "√",
/* 0xFC */ "ⁿ",
/* 0xFD */ "²",
/* 0xFE */ "■",
/* 0xFF */ ""};

#ifdef _WIN32

static bool utf8 = false;

bool con8_utf8() {
    if (!con_istty()) return false;
    utf8 = GetConsoleOutputCP() == 65001;
    return utf8;
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

static bool utf8 = true;

bool con8_utf8() {
    if (!con_istty()) return false;
    setlocale(LC_CTYPE, "");
    utf8 = strcmp(nl_langinfo(CODESET), "UTF-8") == 0;
    return utf8;
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

void utf8_map(bool enable) {
    utf8 = enable;
}

bool con8_mapc(char code) {
    uint8_t ch = (uint8_t)code;
    if (!ch || ch == 0xff) return false;
    if (utf8) {
        if (ch & 0x80U)
            fputs(pcmap[ch & 0x7fU], stdout);
        else
            fputc(code, stdout);
    } else {
        fputc(code, stdout);
    }
    fflush(stdout);
    return true;
}

size_t con8_maps(char *from, size_t cpmax) {
    if (!from) return 0;
    size_t size, count = 0;
    while (count < cpmax && *from && ferror(stdout) == 0) {
        if (!con8_mapc(*from)) break;
        ++count;
    }
    return count;
}

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
