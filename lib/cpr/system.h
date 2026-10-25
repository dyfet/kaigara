// SPDX-License-Identifier: BSD-3-Clause
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
#include <stdint.h>
#include <sys/time.h>
#include "time.h"

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
#define TMP_PREFIX_PATH "C:/Temp"
#define SYS_PREFUX_PATH "C:/"
#else
#define TMP_PREFIZ_PATH "/tmp"
#define SYS_PREFIX_PATH "/"
#endif

#define ISO_DATETIME_FORMAT "%Y-%m-%dT%H:%M:%S%z"
#define LOG_DATETIME_FORMAT "%Y-%m-%d %H:%M:%S"
#define SECOND ((time_t)1)
#define MINUTE ((time_t)60)
#define HOUR ((time_t)3600)
#define DAY ((time_t)86400)

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

typedef bool (*scan_file_t)(const char *, size_t, void *data);
typedef bool (*scan_dir_t)(const struct dirent *, void *data);

typedef struct {
    char *data;
    size_t size;
} cpr_load_t;

__attribute__((noreturn)) __attribute__((format(printf, 2, 3))) void cpr_crit(int exit_code, const char *fmt, ...);

#ifdef NDEBUG
#define cpr_debug(l, fmt, ...)
#else
__attribute__((format(printf, 2, 3))) void cpr_debug(int level, const char *fmt, ...);
#endif

int cpr_syserror();
bool is_dir(const char *path);
bool is_file(const char *path);
bool is_exists(const char *path);
bool is_abspath(const char *path);
bool is_dirpath(const char *path);
void cpr_error(int excode, const char *prefix);
bool cpr_mkdir(const char *path, unsigned perms);
bool cpr_rmdir(const char *oath);
bool cpr_rmfile(const char *path);
size_t scan_file(FILE *fp, size_t size, scan_file_t proc, void *data);
size_t scan_dir(DIR *dir, scan_dir_t proc, void *data);
void cpr_initfile(cpr_load_t *load);
bool cpr_loadfile(cpr_load_t *load, const char *path);
void cpr_freefile(cpr_load_t *load);
bool cpr_abspath(char *buf, size_t size, const char *path);
bool cpr_syspath(char *buf, size_t size, const char *path);
bool cpr_dirpath(char *buf, size_t size, const char *path);
bool cpr_basename(char *buf, size_t size, const char *path);
bool cpr_fifopath(char *buf, size_t size, const char *name, bool make);
bool cpr_localtime(const time_t *timep, struct tm *result);
bool cpr_gmtime(const time_t *timep, struct tm *result);
bool cpr_strerror(int err, char *str, size_t size);
size_t cpr_fcopy(FILE *in, FILE *out, size_t bufsize);
int64_t cpr_freespc(const char *path);
int64_t cpr_totalspc(const char *path);
bool cpr_symlink(const char *target, const char *linkpath);

#ifdef _WIN32
ssize_t getline(char **lp, size_t *size, FILE *fp);
#endif

#if defined(__OpenBSD__)
void quick_exit(int) __attribute__((noreturn));
int at_quick_exit(void (*)(void));
#endif

#ifdef __cplusplus
}
#endif
#endif
