// SPDX-License-Identifier: BSD-3-Clause
// Copyright (C) 2026 David Sugar <tychosoft@gmail.com>

#ifndef LOCAL_SPARSE_H
#define LOCAL_SPARSE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

bool sparse_pagesize(uint16_t page);
bool mksparse(const char *path, size_t size, unsigned perms);
bool dupsparse(const char *from, const char *to, uint16_t page);

#ifdef __cplusplus
}
#endif
#endif
