// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#undef NDEBUG
#include <assert.h>
#include <cpr/endian.h>

int main() {
    const uint8_t bytes[] = {0x01, 0x02};

    assert(be_get16(bytes) == 258U);
    assert(le_get16(bytes) == 513U);
}
