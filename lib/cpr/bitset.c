// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#include "bitset.h"

static inline uint8_t bytemask(size_t bits) {
    return (uint8_t)1 << (bits % 8);
}

static inline int count_bits(uint8_t n) {
    int count = 0;
    while (n > 0) {
        n &= (n - 1);
        count++;
    }
    return count;
}

bits_t cpr_makebits(size_t bitsize) {
    size_t size = cpr_bitsize(bitsize);
    bits_t bits = malloc(size);
    if (bits == NULL) return NULL;
    cpr_memset(bits, 0, size);
    return bits;
}

bits_t cpr_makemask(bits_t bits, bits_t mask, size_t bitsize) {
    if (!bitsize || !bits || !mask) return NULL;
    bits_t out = cpr_makebits(bitsize);
    if (!out) return NULL;
    size_t size = cpr_bitsize(bitsize);
    for (size_t pos = 0; pos < size; ++pos) {
        out[pos] = bits[pos] & mask[pos];
    }
    return out;
}

void cpr_foldmask(bits_t bits, bits_t mask, size_t bitsize) {
    if (!mask || !bitsize) return;
    if (!bits) {
        free(mask);
        return;
    }
    size_t size = cpr_bitsize(bitsize);
    for (size_t pos = 0; pos < size; ++pos) {
        bits[pos] &= ~mask[pos];
    }
    free(mask);
}

void cpr_freebits(bits_t bits) {
    if (bits) free(bits);
}

bool cpr_bitget(bits_t bits, size_t offset) {
    if (!bits) return false;
    size_t bytes = offset / 8;
    uint8_t mask = bytemask(offset);
    return (bits[bytes] & mask) != 0;
}

void cpr_bitset(bits_t bits, size_t offset) {
    if (!bits) return;
    size_t bytes = offset / 8;
    uint8_t mask = bytemask(offset);
    bits[bytes] |= mask;
}

void cpr_bitclr(bits_t bits, size_t offset) {
    if (!bits) return;
    size_t bytes = offset / 8;
    uint8_t mask = bytemask(offset);
    bits[bytes] &= ~mask;
}

bool cpr_anybits(bits_t bits, size_t bitsize) {
    if (!bits) return false;
    size_t size = cpr_bitsize(bitsize);
    while (size--) {
        if (*(bits++)) return true;
    }
    return false;
}

size_t cpr_bitcount(bits_t bits, size_t bitsize) {
    if (!bits || bitsize == 0) return 0;
    size_t total = 0;
    size_t bytes = cpr_bitsize(bitsize);
    for (size_t i = 0; i < bytes; ++i) {
        total += count_bits(bits[i]);
    }
    return total;
}

size_t cpr_lowestclr(bits_t bits, size_t bitsize) {
    if (!bits) return bitsize;
    size_t bytes = cpr_bitsize(bitsize);
    for (size_t i = 0; i < bytes; ++i) {
        if (bits[i] != 0xFF) {
            int bit = __builtin_ffs(~bits[i]) - 1;
            size_t pos = (i * 8) + bit;
            if (pos < bitsize) {
                return pos;
            } else { // we hit padding...
                break;
            }
        }
    }
    return bitsize;
}

size_t cpr_lowestset(bits_t bits, size_t bitsize) {
    if (!bits) return bitsize;
    size_t bytes = cpr_bitsize(bitsize);
    for (size_t i = 0; i < bytes; ++i) {
        if (bits[i] != 0x00) {
            int bit = __builtin_ffs(bits[i]) - 1;
            size_t pos = (i * 8) + bit;
            if (pos < bitsize) {
                return pos;
            } else {
                break;
            }
        }
    }
    return bitsize;
}
