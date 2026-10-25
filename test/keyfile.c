// SPDX-License-Identifier: BSD-3-Clause
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#undef NDEBUG
#include <assert.h>
#include <cpr/keyfile.h>

// for lint
#ifndef TEST_DATA
#define TEST_DATA "."
#endif

int main() {
    keysection_t *root = load_keyfile(TEST_DATA "/test.conf");
    assert(root != NULL);
    keysection_t *section = find_keysection(root, "10");
    assert(section != NULL);
    assert(eq(section->id, "10"));
    assert(eq(get_keyvalue(section, "id"), "userid"));
    assert(eq(get_keyvalue(section, "secret"), "none"));
    assert(NULL == get_keyvalue(section, "XXX"));
    free(root);
    return 0;
}
