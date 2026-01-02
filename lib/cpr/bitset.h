// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#ifndef CPR_BITSET_H
#define CPR_BITSET_H

#include "memory.h"

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint8_t *bits_t;

bits_t cpr_makebits(size_t bitsize);
bits_t cpr_makemask(bits_t bits, bits_t mask, size_t bitsize);
void cpr_foldmask(bits_t bits, bits_t mask, size_t bitsize);
void cpr_freebits(bits_t bits);
bool cpr_bitget(bits_t bits, size_t offset);
void cpr_bitset(bits_t bits, size_t offset);
void cpr_bitclr(bits_t bits, size_t offset);
bool cpr_anybits(bits_t bits, size_t bitsize);
size_t cpr_bitcount(bits_t bits, size_t bitsize);
size_t cpr_lowestclr(bits_t bits, size_t bitsize);
size_t cpr_lowestset(bits_t bits, size_t bitsize);

static inline bool cpr_allbits(bits_t bits, size_t bitsize) {
    return cpr_bitcount(bits, bitsize) == bitsize;
}

static inline size_t cpr_bitsize(size_t bits) {
    size_t result = bits / 8;
    if (bits % 8 != 0) ++result;
    return result;
}

#ifdef __cplusplus
}
#endif
#endif
