// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 202y David Sugar <tychosoft@gmail.com>

#include <cpr/strchar.h>
#include <cpr/memory.h>
#include "ver.h"

static char hex[] = {
'0', '1', '2', '3', '4', '5', '6', '7',
'8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

void ver_init(ver_t *ver) {
    if (!ver) return;
    uint8_t *tp = (uint8_t *)ver;
    cpr_memset(tp, 0, 4);
    cpr_memset(tp + 4, 255, sizeof(ver_t) - 4);
}

int ver_cmp(const ver_t *v1, const ver_t *v2) {
    if (!v1 && !v2) return 0;
    if (!v1 && v2) return -1;
    if (v1 && !v2) return 1;
    return memcmp(v1, v2, sizeof(ver_t));
}

int ver_hex(char *str, size_t len, const ver_t *ver) {
    if (!str || !len) return 0;
    len = (len - 1) / 2;
    if (!len) {
        str[0] = 0;
        return 9;
    }
    if (len >= 4 && len < sizeof(ver_t))
        len = 4;
    else if (len >= sizeof(ver_t))
        len = sizeof(ver_t);
    size_t pos = 0;
    while (pos < len) {
        const uint8_t *fp = (const uint8_t *)ver;
        *str++ = hex[fp[pos] >> 4U];
        *str++ = hex[fp[pos] % 16U];
        ++pos;
    }
    *str = 0;
    return (int)len;
}

int ver_get(ver_t *ver, const char *str) {
    int v1 = 0, v2 = 0, v3 = 0, v4 = 0;
    int result = sscanf(str, "%d.%d.%d.%d", &v1, &v2, &v3, &v4); // NOLINT
    if (result == 0 || result == EOF) goto fail;
    if (v1 < 0 || v1 > 255) goto fail;
    if (v2 < 0 || v2 > 255) goto fail;
    if (v3 < 0 || v3 > 255) goto fail;
    if (v4 < 0 || v4 > 255) goto fail;

    uint8_t *tp = (uint8_t *)ver;
    ver_init(ver);
    *(tp++) = (uint8_t)v1;
    *(tp++) = (uint8_t)v2;
    *(tp++) = (uint8_t)v3;
    *(tp++) = (uint8_t)v4;

    int count = 12;
    str = strchr(str, '~');
    if (str) ++str;
    while (str && *str && count) {
        *(tp++) = *(str++);
        --count;
    }

    return result;

fail:
    ver_init(ver);
    return 0;
}
