// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#include "memio.h"
#include "strchar.h"
#include "memory.h"

#include <stdlib.h>

void cpr_initmem(memio_t *mem, char *from, size_t size) {
    mem->get = mem->put = 0;
    mem->data = from;
    mem->size = size;
    mem->alloc = false;
}

void cpr_resetmem(memio_t *mem) {
    if (!mem) return;
    mem->get = mem->put = 0;
}

void cpr_freemem(memio_t *mem) {
    if (!mem || !mem->alloc) return;
    free(mem);
}

memio_t *cpr_makemem(size_t size) {
    memio_t *mem = malloc(size + sizeof(memio_t));
    if (!mem) return NULL;
    mem->get = mem->put = 0;
    mem->data = mem->buf;
    mem->size = size;
    mem->alloc = true;
    return mem;
}

char cpr_cgetmem(memio_t *mem) {
    if (!mem || mem->get >= mem->size) return 0;
    return mem->data[mem->get++];
}

bool cpr_cputmem(memio_t *mem, char ch) {
    if (!mem || mem->put >= mem->size) return false;
    mem->data[mem->put++] = ch;
    return true;
}

bool cpr_xputmem(memio_t *mem, const char *from, size_t size) {
    if (!mem || (mem->size - mem->put) < size) return false;
    if (!cpr_memcpy(&mem->data[mem->put], mem->size - mem->put, from, size))
        return false;
    mem->put += size;
    return true;
}

bool cpr_sputmem(memio_t *mem, const char *text) {
    if (!text || !mem || *text == 0) return false;
    // +1 so if string too big it is false...
    return cpr_xputmem(mem, text, cpr_strlen(text, mem->size));
}

bool cpr_fmtmem(memio_t *mem, size_t estimated, const char *fmt, ...) {
    if (!mem || !fmt || !estimated || estimated > mem->size) return false;
    if (mem->put + estimated > mem->size) return false;

    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(&mem->data[mem->put], estimated, fmt, ap); // FlawFinder: checked
    va_end(ap);

    if (n < 0 || (size_t)n >= estimated) return false;
    mem->put += (size_t)n;
    return true;
}

const void *cpr_xgetmem(memio_t *mem, size_t size) {
    if (!mem || ((mem->get + size) > mem->size)) return NULL;
    const void *data = &mem->data[mem->get];
    mem->get += size;
    return data;
}

const char *cpr_lgetmem(memio_t *mem, size_t *outlen, const char *delim) {
    if (!mem || mem->get >= mem->size) return NULL;
    if (delim == NULL) delim = "\n";
    size_t delim_len = cpr_strlen(delim, 16);
    while (1) {
        for (size_t i = mem->get; i + delim_len <= mem->size; ++i) {
            if (memcmp(&mem->data[i], delim, delim_len) == 0) {
                const char *result = &mem->data[mem->get];
                size_t len = i - mem->get;
                if (outlen) {
                    *outlen = len;
                } else {
                    if (i < mem->size) {
                        mem->data[i] = 0;
                    } else {
                        return NULL;
                    }
                }
                mem->get = i + delim_len;
                return result;
            }
        }
        return NULL;
    }
}
