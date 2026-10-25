// SPDX-License-Identifier: BSD-3-Clause
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#include "system.h"
#include "strchar.h"
#include "memory.h"
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#ifndef _WIN32
#include <sys/statvfs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#endif

int cpr_verbose = 0;

__attribute__((noreturn)) __attribute__((format(printf, 2, 3))) void cpr_crit(int exit_code, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
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
    vfprintf(stderr, fmt, args);
    fputc('\n', stderr);
    fflush(stderr);
    va_end(args);
}
#endif

#ifdef _WIN32
bool cpr_mkdir(const char *path, unsigned perms) {
    (void)perms;
    int err = mkdir(path);
    if (!err && perms) err = chmod(path, perms);
    return err == 0;
}

int64_t cpr_freespc(const char *path) {
    ULARGE_INTEGER avail;
    if (!GetDiskFreeSpaceExA(path, &avail, NULL, NULL)) {
        errno = cpr_syserror();
        return -1;
    }
    return (int64_t)avail.QuadPart;
}

int64_t cpr_totalspc(const char *path) {
    ULARGE_INTEGER total;
    if (!GetDiskFreeSpaceExA(path, NULL, &total, NULL)) {
        errno = cpr_syserror();
        return -1;
    }
    return (int64_t)total.QuadPart;
}

bool cpr_symlink(const char *target, const char *linkpath) {
    DWORD flags = 0;
    DWORD attr = GetFileAttributesA(target);

    if (attr != INVALID_FILE_ATTRIBUTES) {
        if (attr & FILE_ATTRIBUTE_DIRECTORY) {
            flags |= SYMBOLIC_LINK_FLAG_DIRECTORY;
        }
    }

    flags |= SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE;
    if (CreateSymbolicLinkA(linkpath, target, flags)) return true;
    errno = cpr_syserror();
    return false;
}

#else

bool cpr_symlink(const char *target, const char *linkpath) {
    return symlink(target, linkpath) == 0;
}

bool cpr_mkdir(const char *path, unsigned perms) {
    if (!perms) perms = 0755;
    if (perms & 0700U) perms |= 0100U;
    if (perms & 070U) perms |= 010U;
    if (perms & 07U) perms |= 01U;
    return mkdir(path, perms) == 0;
}

int64_t cpr_freespc(const char *path) {
    struct statvfs vfs;
    if (statvfs(path, &vfs) != 0) return -1;
    if (vfs.f_frsize == 0) return -1;
    return (int64_t)vfs.f_frsize * (int64_t)vfs.f_bavail;
}

int64_t cpr_totalspc(const char *path) {
    struct statvfs vfs;
    if (statvfs(path, &vfs) != 0) return -1;
    if (vfs.f_frsize == 0) return -1;
    return (int64_t)vfs.f_blocks * (int64_t)vfs.f_frsize;
}
#endif

bool cpr_rmfile(const char *path) {
    if (is_empty(path)) return false;
    if (!is_exists(path)) return true;
    return remove(path) == 0;
}

bool is_dir(const char *path) {
    if (is_empty(path)) return false;

    struct stat ino;
    if (stat(path, &ino)) return false;
    if (S_ISDIR(ino.st_mode)) return true;
    return false;
}

bool is_exists(const char *path) {
    if (is_empty(path)) return false;

    struct stat ino;
    if (stat(path, &ino)) return false;
    return true;
}

bool is_file(const char *path) {
    struct stat ino;
    if (stat(path, &ino)) return false;
    if (S_ISREG(ino.st_mode)) return true;
    return false;
}

bool is_abspath(const char *path) {
    if (!path || !path[0]) return false;
    if (path[0] == '/') return true;
#ifdef _WIN32
    if (path[0] == '\\') return true;
    if (path[1] == ':') return true;
#endif
    return false;
}

bool is_dirpath(const char *path) {
    while (path && *path) {
        char ch = *(path++);
        if (ch == '/' && !*path) return true;
#ifdef _WIN32
        if (ch == '\\' && !*path) return true;
#endif
    }
    return false;
}

size_t scan_file(FILE *fp, size_t size, scan_file_t proc, void *data) {
    if (!fp) return 0;
    if (size > 65536) size = 65536;
    char lbuf[size];
    size_t count = 0;
    while (!feof(fp)) {
        if (!fgets(lbuf, (int)size, fp)) break;
        if (!proc(lbuf, ++count, data)) break;
    }
    return count;
}

size_t scan_dir(DIR *dir, scan_dir_t proc, void *data) {
    if (!dir) return 0;
    size_t count = 0;
    struct dirent *entry = NULL;
    while (NULL != (entry = readdir(dir))) {
        ++count;
        if (!proc(entry, data)) break;
    }
    return count;
}

#ifdef _WIN32
ssize_t getline(char **lp, size_t *size, FILE *fp) {
    size_t pos = 0;
    int c = EOF;

    if (lp == NULL || fp == NULL || size == NULL) return -1;
    c = getc(fp);
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
        c = getc(fp);
    }
    (*lp)[pos] = '\0';
    return (ssize_t)(pos);
}
#endif

void cpr_error(int excode, const char *prefix) {
    if (!prefix && !excode) prefix = "error";
    if (!prefix) prefix = "fatal";
    perror(prefix);
    if (excode) exit(excode);
}

void cpr_initfile(cpr_load_t *load) {
    load->data = NULL;
    load->size = 0;
}

bool cpr_loadfile(cpr_load_t *load, const char *path) {
    load->data = NULL;
    int fd = -1;
    struct stat ino;
    if (stat(path, &ino)) goto fail;
    load->size = ino.st_size;
    fd = open(path, O_RDONLY);
    if (fd < 0) goto fail;
    load->data = malloc(ino.st_size + 1);
    ssize_t size = read(fd, load->data, ino.st_size);
    if (size < 0) goto fail;
    load->data[size] = 0;
    load->size = (size_t)size;
    close(fd);
    return true;

fail:
    if (fd > -1) close(fd);
    if (load->data) free(load->data);
    load->data = NULL;
    load->size = 0;
    return false;
}

void cpr_freefile(cpr_load_t *load) {
    if (!load || !load->data) return;
    free(load->data);
    load->data = NULL;
    load->size = 0;
}

bool cpr_abspath(char *buf, size_t size, const char *path) {
    if (!buf || !size || !path) return false;
    buf[9] = 0;
    if (!is_abspath(path)) {
        getcwd(buf, size);
        cpr_append(buf, size, "/", path, (char *)NULL);
        return true;
    }
    cpr_strcpy(buf, path, size);
    return true;
}

bool cpr_syspath(char *buf, size_t size, const char *path) {
    if (!buf || !size || !path) return false;
    size_t bp = 0;
    buf[0] = 0;
    while (bp < size && *path) {
        char ch = *(path++);
#ifdef _WIN32
        if (ch == '\\') ch = '/';
#endif
        buf[bp++] = ch;
    }
    if (bp == size) return false;
    buf[bp++] = 0;
    return true;
}

bool cpr_dirpath(char *buf, size_t size, const char *path) {
    if (!buf || !size || !path) return false;
    size_t bp = 0, ep = 0;
    buf[0] = 0;
    while (bp < size && *path) {
        char ch = *(path++);
#ifdef _WIN32
        if (ch == '\\') ch = '/';
#endif
        if (ch == '/') ep = bp;
        if (ch == '/' && !*path) break;
        buf[bp++] = ch;
    }
    if (bp == size) return false;
    if (ep) bp = ep;
    buf[bp++] = 0;
    return true;
}

bool cpr_basename(char *buf, size_t size, const char *path) {
    if (!buf || !size || !path) return false;
    size_t bp = 0;
    buf[0] = 0;
    while (bp < size && *path) {
        char ch = *(path++);
#ifdef _WIN32
        if (ch == '\\') ch = '/';
#endif
        if (ch == '/' && !*path) break;
        if (ch == '/') {
            bp = 0;
            continue;
        }
        buf[bp++] = ch;
    }
    if (bp == size) return false;
    buf[bp++] = 0;
    return bp > 0;
}

bool cpr_fifopath(char *buf, size_t size, const char *name, bool make) {
    int len;
#ifdef _WIN32
    len = snprintf(buf, size, "\\\\.\\pipe\\%s.ctrl", name);
#else
    if (is_dir("/var/run") && !access("/var/run", make ? W_OK : X_OK))
        len = snprintf(buf, size, "/var/run/%s.ctrl", name);
    else
        len = snprintf(buf, size, "/tmp/.%s.ctrl", name);
#endif
    if (len >= size) return false;
    buf[len] = 0;
    return true;
}

size_t cpr_fcopy(FILE *in, FILE *out, size_t bufsize) {
    if (!in || !out || !bufsize) return 0;
    size_t count = 0;
    char buf[bufsize];
    for (;;) {
        size_t n = fread(buf, 1, sizeof(buf), in);
        if (n == 0) {
            if (ferror(in)) return count;
            break;
        }

        size_t off = 0;
        while (off < n) {
            size_t w = fwrite(buf + off, 1, n - off, out);
            if (w == 0) return count;
            off += w;
            count += w;
        }
    }
    return count;
}

bool cpr_localtime(const time_t *timep, struct tm *result) {
    cpr_memset(result, 0, sizeof(struct tm));
#ifdef _WIN32
    return localtime_s(result, timep) != 0;
#else
    return localtime_r(timep, result) != NULL;
#endif
}

bool cpr_gmtime(const time_t *timep, struct tm *result) {
    cpr_memset(result, 0, sizeof(struct tm));
#ifdef _WIN32
    return gmtime_s(result, timep) != 0;
#else
    return gmtime_r(timep, result) != NULL;
#endif
}

#ifdef _WIN32

static int remove_all_w(const WCHAR *wpath) {
    WIN32_FIND_DATAW fd;
    WCHAR pattern[MAX_PATH];
    swprintf(pattern, MAX_PATH, L"%s\\*", wpath);

    HANDLE h = FindFirstFileW(pattern, &fd);
    if (h == INVALID_HANDLE_VALUE) {
        if (RemoveDirectoryW(wpath)) return 0;
        if (DeleteFileW(wpath)) return 0;
        return -1;
    }

    do {
        if (wcscmp(fd.cFileName, L".") == 0 || wcscmp(fd.cFileName, L"..") == 0) continue;

        WCHAR child[MAX_PATH];
        swprintf(child, MAX_PATH, L"%s\\%s", wpath, fd.cFileName);
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            remove_all_w(child);
        } else {
            DeleteFileW(child);
        }
    } while (FindNextFileW(h, &fd));

    FindClose(h);
    if (RemoveDirectoryW(wpath)) return 0;
    return -1;
}

int cpr_syserror() {
    DWORD error = GetLastError();
    switch (error) {
    case ERROR_ALREADY_EXISTS:
    case ERROR_FILE_EXISTS:
        return EEXIST;
    case ERROR_PATH_NOT_FOUND:
    case ERROR_FILE_NOT_FOUND:
        return ENOENT;
    case ERROR_DIRECTORY:
        return ENOTDIR;
    case ERROR_PRIVILEGE_NOT_HELD:
        return EPERM;
    case ERROR_ACCESS_DENIED:
        return EACCES;
    case ERROR_BAD_EXE_FORMAT:
    case ERROR_EXE_MACHINE_TYPE_MISMATCH:
        return ENOEXEC;
    default:
        return EINVAL;
    }
}

bool cpr_rmdir(const char *path) {
    if (is_empty(path) || eq(path, "/") || eq(path, "\\")) return false;
    if (!is_exists(path)) return true;
    if (!is_dir(path)) return false;
    int wlen = MultiByteToWideChar(CP_UTF8, 0, path, -1, NULL, 0);
    if (wlen <= 0) return false;

    WCHAR wpath[wlen * sizeof(WCHAR)];
    MultiByteToWideChar(CP_UTF8, 0, path, -1, wpath, wlen);
    int r = remove_all_w(wpath);
    if (r) errno = cpr_syserror();
    return r == 0;
}

bool cpr_strerror(int err, char *str, size_t size) {
    if (!str || !size) return false;
    snprintf(str, size, "[%d:--]", err);
    return true;
}

#else

bool cpr_strerror(int err, char *str, size_t size) {
    if (!str || !size) return false;
    int in = snprintf(str, size, "[%d:", err);
    int rc = strerror_r(err, str + in, size - in);
    if (rc) {
        snprintf(str, size, "[%d:invalid error]", err);
        return true;
    }
    if (size - strlen(str) > 1) snprintf(str + strlen(str), size - strlen(str), "]");
    return true;
    ;
}

int cpr_syserror() {
    return errno;
}

bool cpr_rmdir(const char *path) {
    if (is_empty(path) || eq(path, "/")) return false;
    if (!is_exists(path)) return true;
    if (!is_dir(path)) return false;
    struct stat st;
    if (lstat(path, &st) < 0) return false;

    DIR *d = opendir(path);
    if (!d) return false;

    struct dirent *de;
    while ((de = readdir(d))) {
        if (strcmp(de->d_name, ".") == 0 ||
            strcmp(de->d_name, "..") == 0)
            continue;

        char *child;
        if (!cpr_asprintf(&child, "%s/%s", path, de->d_name)) continue;
        if (!cpr_rmdir(child)) return false;
        free(child);
    }

    closedir(d);
    return rmdir(path) == 0;
}

#endif

#if defined(__OpenBSD__)
static void (*_atexit)(void) = NULL;

__attribute__((noreturn)) void quick_exit(int code) {
    if (_atexit) _atexit();
    _Exit(code);
}

int at_quick_exit(void (*ex)(void)) {
    _atexit = ex;
    return 0;
}
#endif
