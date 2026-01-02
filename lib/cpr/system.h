// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#ifndef CPR_SYSTEM_H
#define CPR_SYSTEM_H

#include <stdbool.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "time.h"
#include "sys/time.h"

#ifdef _WIN32
#define MICROSOFT_WINDOWS_WINBASE_H_DEFINE_INTERLOCKED_CPLUSPLUS_OVERLOADS 0
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <tlhelp32.h>
#include <io.h>
#include <direct.h>
#include <winioctl.h>
#include <fileapi.h>
#include <string.h>
#endif

#define ISO_DATETIME_FORMAT "%Y-%m-%dT%H:%M:%S%z"
#define LOG_DATETIME_FORMAT "%Y-%m-%d %H:%M:%S"

#if defined(__OpenBSD__)
#define stat64 stat
#define fstat64 fstat
#endif

#ifndef R_OK
#define F_OK 0
#define X_OK 1
#define W_OK 2
#define R_OK 4
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern int cpr_verbose;

__attribute__((noreturn)) __attribute__((format(printf, 2, 3))) void cpr_crit(int exit_code, const char *fmt, ...);

#ifdef NDEBUG
#define cpr_debug(l, fmt, ...)
#else
__attribute__((format(printf, 2, 3))) void cpr_debug(int level, const char *fmt, ...);
#endif

int make_dir(const char *path, int perms);
bool is_dir(const char *path);
bool is_file(const char *path);
size_t scan_file(FILE *fp, size_t size, bool (*proc)(const char *, size_t));
size_t scan_dir(DIR *dir, bool (*proc)(const struct dirent *));

#ifdef _WIN32
ssize_t getline(char **lp, size_t *size, FILE *fp);
#endif

#ifdef __cplusplus
}
#endif
#endif
