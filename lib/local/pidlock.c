// SPDX-License-Identifier: BSD-3-Clause
// Copyright (C) 2026 David Sugar <tychosoft@gmail.com>

#include "pidlock.h"
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32

#include <windows.h>

static int syserror() {
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
    default:
        return EINVAL;
    }
}

#else

#include <fcntl.h>
#include <unistd.h>

#endif

bool pidlock_acquire(pidlock_t *lock, const char *path, bool blocking) {
    if (!lock || !path || !*path) return false;
    memset(lock, 0, sizeof(pidlock_t));
    strncpy(lock->path, path, sizeof(lock->path) - 1);

#ifdef _WIN32
    HANDLE handle = CreateFileA(path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (handle == INVALID_HANDLE_VALUE) {
        errno = syserror();
        return false;
    }

    OVERLAPPED ov = {0};
    DWORD flags = LOCKFILE_EXCLUSIVE_LOCK;
    if (!blocking) flags |= LOCKFILE_FAIL_IMMEDIATELY;
    if (!LockFileEx(handle, flags, 0, 1, 0, &ov)) {
        errno = syserror();
        CloseHandle(handle);
        return false;
    }

    lock->file = (intptr_t)handle;
    lock->locked = true;
#else
    int fd = open(path, O_RDWR | O_CREAT, 0666);
    if (fd < 0) return false;

    int cmd = blocking ? F_SETLKW : F_SETLK;
    struct flock flock;
    memset(&flock, 0, sizeof(flock));
    flock.l_type = F_WRLCK;
    flock.l_whence = SEEK_SET;
    flock.l_start = 0;
    flock.l_len = 0;
    if (fcntl(fd, cmd, &flock) < 0) {
        close(fd);
        return false;
    }
    lock->file = fd;
    lock->locked = true;
#endif
    if (!pidlock_update(lock, NULL)) {
        pidlock_release(lock);
        return false;
    }
    return true;
}

bool pidlock_release(pidlock_t *lock) {
    if (!lock || !lock->locked) return false;
#ifdef _WIN32
    LARGE_INTEGER size;
    size.QuadPart = 1;
    SetFilePointerEx((HANDLE)lock->file, size, NULL, FILE_BEGIN);
    SetEndOfFile((HANDLE)lock->file);
    OVERLAPPED ov = {0};
    UnlockFileEx((HANDLE)lock->file, 0, 1, 0, &ov);
    CloseHandle((HANDLE)lock->file);
    lock->locked = false;
#else
    ftruncate((int)lock->file, 0);
    struct flock flock;
    memset(&flock, 0, sizeof(flock));
    flock.l_type = F_UNLCK;
    flock.l_whence = SEEK_SET;
    flock.l_start = 0;
    flock.l_len = 0;
    fcntl((int)lock->file, F_SETLK, &flock);
    close((int)lock->file);
    lock->locked = false;
#endif
    return true;
}

void pidlock_close(pidlock_t *lock) {
    if (!lock || !lock->locked) return;
#ifdef _WIN32
    CloseHandle((HANDLE)lock->file);
    lock->locked = false;
#else
    close((int)lock->file);
    lock->locked = false;
#endif
}

void pidlock_cleanup(pidlock_t *lock) {
    if (!lock || !lock->locked) return;
    pidlock_release(lock);
    remove(lock->path);
}

bool pidlock_update(pidlock_t *lock, const char *status) {
    if (!lock || !lock->locked) return false;
    char buf[128];
    int len;
#ifdef _WIN32
    DWORD written = 0;
    if (status)
        len = snprintf(buf, sizeof(buf), "%lu %s\n", GetCurrentProcessId(), status);
    else
        len = snprintf(buf, sizeof(buf), "%lu\n", GetCurrentProcessId());

    if (!SetFilePointer((HANDLE)lock->file, 1, NULL, FILE_BEGIN) || !WriteFile((HANDLE)lock->file, buf, len, &written, NULL)) {
        errno = syserror();
        return false;
    }
#else
    if (status)
        len = snprintf(buf, sizeof(buf), "%d %s\n", getpid(), status);
    else
        len = snprintf(buf, sizeof(buf), "%d\n", getpid());
    if (lseek((int)lock->file, 1, SEEK_SET) < 0) return false;
    if (write((int)lock->file, buf, len) != len) return false;
#endif
    return true;
}

long pidlock_process(const char *path) {
    if (!path || !*path) return false;
    char buf[34];

#ifdef _WIN32
    HANDLE handle = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (handle == INVALID_HANDLE_VALUE) {
        errno = syserror();
        return -1;
    }

    DWORD read = 0;
    if (!SetFilePointer(handle, 1, NULL, FILE_BEGIN) || !ReadFile(handle, buf, sizeof(buf) - 1, &read, NULL) || !read) {
        CloseHandle(handle);
        return 0;
    }
    buf[read] = 0;
#else
    int fd = open(path, O_RDONLY);
    if (fd < 0) return -1;
    if (lseek(fd, 1, SEEK_SET) < 0) {
        close(fd);
        return -1;
    }

    ssize_t n = read(fd, buf, sizeof(buf) - 1);
    close(fd);
    if (n == 0) return 0;
    if (n < 0) return -1;
    buf[n] = 0;
#endif
    char *cp = strchr(buf, ' ');
    if (cp) *cp = 0;
    return atol(buf); // NOLINT
}
