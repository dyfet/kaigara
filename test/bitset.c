// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#undef NDEBUG
#include <assert.h>
#include <cpr/bitset.h>
#include <stdio.h>

int main() {
    bits_t bits = cpr_makebits(80);
    assert(bits != NULL);
    assert(cpr_bitcount(bits, 80) == 0);
    assert(cpr_anybits(bits, 80) == false);
    cpr_bitset(bits, 13);
    assert(cpr_lowestset(bits, 80) == 13);
    cpr_bitset(bits, 3);
    assert(cpr_lowestset(bits, 80) == 3);
    assert(cpr_bitget(bits, 13) == true);
    assert(cpr_bitget(bits, 14) == false);
    assert(cpr_anybits(bits, 80) == true);
    assert(cpr_bitcount(bits, 80) == 2);
    cpr_bitclr(bits, 13);
    assert(cpr_bitcount(bits, 80) == 1);
    assert(cpr_lowestclr(bits, 80) == 0);
    cpr_bitset(bits, 0);
    assert(cpr_lowestclr(bits, 80) == 1);
    assert(cpr_lowestset(bits, 80) == 0);
    cpr_freebits(bits);
    assert(cpr_bitsize(81) == 11);
    return 0;
}
