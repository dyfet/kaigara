// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#ifndef LOCAL_SHELL_H
#define LOCAL_SHELL_H

#include <stddef.h>
#include <stdbool.h>
#include "scan.h"

#ifdef _WIN32
#define DEV_SIZE 19
#define DEV_NULL "NUL"
#else
#define DEV_SIZE 32
#define DEV_NULL "/dev/null"
#endif

bool dev_name(const char *from, char *buf, size_t size);
bool fix_path(char *cp, bool dir);
bool flag_one_none(unsigned count, ...);
bool flag_one_only(unsigned count, ...);

#endif
