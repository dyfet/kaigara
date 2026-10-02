// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#undef NDEBUG
#include <assert.h>
#include <cpr/memory.h>
#include <stdio.h>

int main() {
    int *mem = NULL;
    memshare_t ints = NULL;

    NEWP(mem, int);
    assert(mem != NULL);
    FREEP(mem);
    assert(mem == NULL);

    MAKE_REF(&ints, int);
    *REF(ints, int) = 7;
    MAKE_REF(&ints, int);
    assert(cpr_count(ints) == 2);
    RELEASE_REF(&ints);
    assert(cpr_count(ints) == 1);
    assert(*REF(ints, int) == 7);
    RELEASE_REF(&ints);
    assert(ints == NULL);
}
