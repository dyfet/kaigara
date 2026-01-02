// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#include "memory.h"

static const char *hex = "0123456789abcdef";

size_t cpr_hexload(uint8_t *out, const char *str, size_t size) {
    if (!out || !size || !str) return 0;
    size_t count = 0;
    while (count < size && *str) {
        const char *cp1 = strchr(hex, tolower(str[0]));
        const char *cp2 = strchr(hex, tolower(str[1]));
        if (!cp1 || !cp2) break;
        out[count++] = ((uint8_t)(cp1 - hex) << 4) | (uint8_t)(cp2 - hex);
        str += 2;
    }
    return count;
}

char *cpr_hexdup(const uint8_t *bin, size_t size) {
    if (!bin) return NULL;
    char *out = malloc((size * 2) + 1);
    if (!out) return NULL;
    char *put = out;
    while (size--) {
        *(put++) = hex[*bin >> 4];
        *(put++) = hex[*bin & 0x0f];
        ++bin;
    }
    *put = 0;
    return out;
}

void *cpr_new(void **ptr, size_t size) {
    if (*ptr)
        free(*ptr);
    *ptr = malloc(size);
    return *ptr;
}

void cpr_free(void **ptr) {
    if (!*ptr)
        return;
    free(*ptr);
    *ptr = NULL;
}

bool cpr_memcpy(void *dst, size_t dst_size, const void *src, size_t src_size) {
    if (!dst || !src) return false;
    if (src_size > dst_size) return false;
    memcpy(dst, src, src_size); // FlawFinder: ignore
    return true;
}

void *cpr_memset(void *ptr, int value, size_t size) {
    volatile uint8_t *volatile p = (volatile uint8_t *volatile)ptr;
    if (!p) return NULL;
    while (size--) {
        *p++ = (uint8_t)value;
    }
    return ptr;
}

memshare_t cpr_makeref(size_t size) {
    memshare_t ptr = (memshare_t)malloc(sizeof(struct _memshare) + size);
    if (!ptr) return NULL;
    atomic_init(&ptr->refcount, 1);
    return ptr;
}

void *cpr_ref(memshare_t ptr) {
    return ((uint8_t *)ptr) + sizeof(struct _memshare);
}

unsigned cpr_count(memshare_t ptr) {
    return atomic_load(&ptr->refcount);
}

memshare_t cpr_retain(memshare_t ptr) {
    atomic_fetch_add(&ptr->refcount, 1);
    return ptr;
}

memshare_t cpr_release(memshare_t ptr) {
    if (atomic_fetch_sub(&ptr->refcount, 1) == 1) {
        free(ptr);
        return NULL;
    }
    return ptr;
}
