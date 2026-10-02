// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 202y David Sugar <tychosoft@gmail.com>

#include "utf8.h"
#include <stdio.h>
#include <string.h>

#if UTF8_HEAP > 0
#include <stdlib.h>
#endif

bool is_asc7(utf8_t from) {
    if (!from) return false;
    const char *cp = (const char *)from;
    while (*cp) {
        unsigned char ch = (unsigned char)*cp;
        if (ch >= 127) return false;
        ++cp;
    }
    return true;
}

bool is_utf8(utf8_t from) {
    if (!from) return false;
    size_t size;
    while (*from) {
        size = utf8_size(from);
        if (!size) return false;
        if (utf8_code(from) == EOF) return false;
        from += size;
    }
    return true;
}

bool is_asc8(utf8_t from) {
    if (!from) return false;
    if (is_asc7(from)) return true;
    return !is_utf8(from);
}

size_t utf8_size(utf8_t from) {
    if (!from) return 0;

    unsigned char v = (unsigned char)(*from);
    if (v < 0x80U) return 1;
    if ((v & 0xe0U) == 0xc0U) return 2;
    if ((v & 0xf0U) == 0xe0U) return 3;
    if ((v & 0xf8U) == 0xf0U) return 4;
    return 0;
}

ucs4_t utf8_code(utf8_t from) {
    if (!from) return EOF;

    size_t size = utf8_size(from);
    unsigned char encoded = (unsigned char)(*(from++));
    ucs4_t code = 0;
    if (!size) return EOF;
    if (!encoded) return 0;
    switch (size) {
    case 1:
        return (ucs4_t)encoded;
    case 2:
        code = (ucs4_t)(encoded & 0x1fU);
        break;
    case 3:
        code = (ucs4_t)(encoded & 0x0fU);
        break;
    case 4:
        code = (ucs4_t)(encoded & 0x07U);
        break;
    default:
        return EOF;
    }

    while (--size) {
        encoded = (unsigned char)(*(from++));
        if ((encoded & 0xc0U) != 0x80U) return EOF;
        code = (ucs4_t)(((uint32_t)code << 6U) | (uint32_t)(encoded & 0x3fU));
    }
    return code;
}

size_t utf8_count(utf8_t from) {
    if (!from) return 0;
    size_t size, total = 0;
    while (*from) {
        size = utf8_size(from);
        if (!size) return 0;
        ++total;
        from += size;
    }
    return total;
}

size_t utf8_prefix(char code) {
    return utf8_size(&code);
}

bool utf8_limit(utf8_t from, size_t limit) {
    if (!from) return false;
    size_t size, total = 0;
    while (*from && total < limit) {
        size = utf8_size(from);
        if (!size) return false;
        if (utf8_code(from) == EOF) return false;
        ++total;
        from += size;
    }
    return (!*from) && (total <= limit);
}

utf8_t utf8_index(utf8_t from, size_t index) {
    if (!from) return NULL;
    size_t size;
    while (*from && index) {
        size = utf8_size(from);
        if (!size) return 0;
        --index;
        from += size;
    }
    if (*from) return from;
    return NULL;
}

size_t utf8_chars(ucs4_t codepoint, utf8_t to) {
    if (!to || codepoint < 0) return 0;
    if (codepoint < 128) {
        to[0] = (char)codepoint;
        return 1;
    }

    uint32_t code = (uint32_t)codepoint;
    unsigned char *encode = (unsigned char *)to;
    if (code < 0x000007ff) {
        encode[0] = (uint8_t)((code >> 6U) | 0xc0U);
        encode[1] = (uint8_t)((code & 0x3fU) | 0x80U);
        return 2;
    }

    if (code <= 0x0000ffff) {
        encode[0] = (uint8_t)((code >> 12U) | 0xe0U);
        encode[1] = (uint8_t)((code >> 6U & 0x3fU) | 0x80U);
        encode[2] = (uint8_t)((code & 0x3fU) | 0x80U);
        return 3;
    }

    if (code <= 0x001fffff) {
        encode[0] = (uint8_t)((code >> 18U) | 0xf0U);
        encode[1] = (uint8_t)((code >> 12U & 0x3fU) | 0x80U);
        encode[2] = (uint8_t)((code >> 6U & 0x3fU) | 0x80U);
        encode[3] = (uint8_t)((code & 0x3fUL) | 0x80UL);
        return 4;
    }
    return 0;
}

size_t utf8_expand(utf8_t from, ucs4_p to, size_t size) {
    if (!from || !to) return 0;
    size_t count = 0;
    while (count < size && *from) {
        size_t u8size = utf8_size(from);
        if (!u8size) break;
        ucs4_t code = utf8_code(from);
        *(to++) = code;
        from += u8size;
        if (!code) break;
        ++count;
    }
    if (size) *to = 0;
    return count;
}

size_t utf8_encode(utf8_t to, ucs4_p from, size_t size) {
    if (!from || !to) return 0;
    size_t total = 0;
    while (size >= 4 && *from) {
        size_t u8size = utf8_chars(*(from++), to);
        if (u8size == 0) break;
        to += u8size;
        total += u8size;
    }
    if (size >= 4) *to = 0;
    return total;
}

#if UTF8_HEAP > 0
void utf8_free(utf8_t str) {
    if (str) free(str);
}

utf8_t utf8_copy(utf8_t from, size_t codes) {
    if (!from || !codes) return NULL;
    size_t size, bytes = 0;
    const char *old = (const char *)from;
    while (*from && codes--) {
        size = utf8_size(from);
        if (!size) return NULL;
        if (utf8_code(from) == EOF) return NULL;
        bytes += size;
        from += size;
    }
    utf8_t new = (utf8_t)malloc(bytes + 1);
    if (!new) return NULL;
    memcpy((char *)new, old, bytes);
    new[bytes] = 0;
    return new;
}
#endif
