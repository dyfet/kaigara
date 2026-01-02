// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#include "system.h"
#include "strchar.h"

int cpr_verbose = 0;

__attribute__((noreturn)) __attribute__((format(printf, 2, 3))) void cpr_crit(int exit_code, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args); // FlawFinder: ignore
    fputc('\n', stderr);
    fflush(stderr);
    va_end(args);
    quick_exit(exit_code);
}

#ifndef NDEBUG
__attribute__((format(printf, 2, 3))) void cpr_debug(int level, const char *fmt, ...) {
    if (cpr_verbose < level) return;
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args); // FlawFinder: ignore
    fputc('\n', stderr);
    fflush(stderr);
    va_end(args);
}
#endif

#ifdef _WIN32
int make_dir(const char *path, int perms) {
    int err = mkdir(path);
    if (!err && perms)
        err = chmod(path, perms); // FlawFinder: ignore
    return err;
}
#else
int make_dir(const char *path, int perms) {
    if (!perms)
        perms = 0755;

    return mkdir(path, perms);
}
#endif

bool is_dir(const char *path) {
    struct stat ino;
    if (stat(path, &ino))
        return false;

    if (S_ISDIR(ino.st_mode))
        return true;

    return false;
}

bool is_file(const char *path) {
    struct stat ino;
    if (stat(path, &ino))
        return false;

    if (S_ISREG(ino.st_mode))
        return true;

    return false;
}

size_t scan_file(FILE *fp, size_t size, bool (*proc)(const char *, size_t)) {
    if (!fp) return 0;
    if (size > 65536) size = 65536;
    char lbuf[size];
    size_t count = 0;
    while (!feof(fp)) {
        if (!fgets(lbuf, (int)size, fp)) break;
        if (!proc(lbuf, ++count)) break;
    }
    return count;
}

size_t scan_dir(DIR *dir, bool (*proc)(const struct dirent *)) {
    if (!dir) return 0;
    size_t count = 0;
    struct dirent *entry = NULL;
    while (NULL != (entry = readdir(dir))) {
        ++count;
        if (!proc(entry)) break;
    }
    return count;
}

#ifdef _WIN32
ssize_t getline(char **lp, size_t *size, FILE *fp) {
    size_t pos = 0;
    int c = EOF;

    if (lp == NULL || fp == NULL || size == NULL) return -1;
    c = getc(fp); // FlawFinder: ignore
    if (c == EOF) return -1;
    if (*lp == NULL) {
        *lp = malloc(128);
        if (*lp == NULL) return -1;
        *size = 128;
    }

    pos = 0;
    while (c != EOF) {
        if (pos + 1 >= *size) {
            size_t new_size = *size + (*size >> 2);
            if (new_size < 128) {
                new_size = 128;
            }
            char *new_ptr = realloc(*lp, new_size);
            if (new_ptr == NULL) return -1;
            *size = new_size;
            *lp = new_ptr;
        }

        ((unsigned char *)(*lp))[pos++] = (unsigned char)c;
        if (c == '\n') break;
        c = getc(fp); // FlawFinder: ignore
    }
    (*lp)[pos] = '\0';
    return (ssize_t)(pos);
}
#endif
