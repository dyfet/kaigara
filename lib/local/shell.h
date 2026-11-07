// SPDX-License-Identifier: BSD-3-Clause
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#ifndef LOCAL_SHELL_H
#define LOCAL_SHELL_H

#include <stddef.h>
#include <stdbool.h>
#include "scan.h"

#ifdef _WIN32
#define DEV_SIZE 19
#define DEV_NULL "NUL"
#define PATH_SEP '\\'
#define LIST_SEP ';'
#define CMD_SHELL "cmd"
#define CMD_OPTION "/c"
#else
#define DEV_SIZE 32
#define DEV_NULL "/dev/null"
#define PATH_SEP '/'
#define LIST_SEP ':'
#define CMD_SHELL "/bin/sh"
#define CMD_OPTION "-c"
#endif

bool dev_name(const char *from, char *buf, size_t size);
bool fix_path(char *cp, bool dir);
bool lck_path(const char *from, char *buf, size_t size);
bool flag_one_none(unsigned count, ...);
bool flag_one_only(unsigned count, ...);
bool flag_one_more(unsigned count, ...);

#endif
