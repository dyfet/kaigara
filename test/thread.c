// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#undef NDEBUG
#include <assert.h>
#include <cpr/thread.h>

static void test_mutex() {
    mtx_t mutex;
    mtx_init(&mutex, mtx_plain);
    mtx_destroy(&mutex);
}

int main() {
    test_mutex();
    return 0;
}
