// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#undef NDEBUG
#include <assert.h>

#include <cpr/memio.h>
#include <cpr/strchar.h>

int main() {
    char *text = strdup("hello: world\r\nversion: 1\r\n\r\n");
    memio_t memio;
    cpr_initmem(&memio, text, strlen(text));
    cpr_xgetmem(&memio, 2);
    const char *l1 = cpr_lgetmem(&memio, NULL, "\r\n");
    const char *l2 = cpr_lgetmem(&memio, NULL, "\r\n");
    const char *l3 = cpr_lgetmem(&memio, NULL, "\r\n");
    const char *l4 = cpr_lgetmem(&memio, NULL, "\r\n");
    assert(l4 == NULL);
    assert(eq(l1, "llo: world"));
    assert(eq(l2, "version: 1"));
    assert(*l3 == 0);
}
