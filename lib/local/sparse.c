// SPDX-License-Identifier: BSD-3-Clause
// Copyright (C) 2026 David Sugar <tychosoft@gmail.com>

#include "sparse.h"

#define SECTOR 512U
#define CLUSTER 16384U

static bool is_sparse(const char *mem, size_t size) {
    if (!mem || size == 0) return true;
    while (size--) {
        if (mem[size]) return false;
    }
    return true;
}

bool sparse_pagesize(uint16_t page) {
    if (page < SECTOR || page > CLUSTER || page % SECTOR) return false;
    return true;
}

#ifdef _WIN32

#include <windows.h>
#include <winioctl.h>
#include <stdint.h>

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

bool mksparse(const char *path, size_t size, unsigned perms) {
    (void)perms;
    if (!path || !size) return false;

    bool result = false;
    HANDLE h = CreateFileA(path, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) {
        errno = syserror();
        return false;
    }

    DWORD tmp;
    if (!DeviceIoControl(h, FSCTL_SET_SPARSE, NULL, 0, NULL, 0, &tmp, NULL)) goto end;

    LARGE_INTEGER li;
    li.QuadPart = (LONGLONG)size;
    if (!SetFilePointerEx(h, li, NULL, FILE_BEGIN)) goto end;
    if (!SetEndOfFile(h)) goto end;
    result = true;

end:
    if (!result) errno = syserror();
    CloseHandle(h);
    return result;
}

bool dupsparse(const char *from, const char *to, uint16_t page) {
    if (!from || !to || !sparse_pagesize(page)) return false;

    bool result = false;
    char buf[page];
    HANDLE in = INVALID_HANDLE_VALUE, out = INVALID_HANDLE_VALUE;
    in = CreateFileA(from, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (in == INVALID_HANDLE_VALUE) goto end;
#include <windows.h>
    FILE_STANDARD_INFO info;
    if (!GetFileInformationByHandleEx(in, FileStandardInfo, &info, sizeof(info))) goto end;
    if (!mksparse(to, info.EndOfFile.QuadPart, 0)) goto end;
    out = CreateFileA(to, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (out == INVALID_HANDLE_VALUE) goto end;

    DWORD count = 0, pos = 0;
    for (;;) {
        if (!ReadFile(in, buf, sizeof(buf), &count, NULL)) goto end;
        if (count == 0) break;
        if (is_sparse(buf, count)) {
            LARGE_INTEGER li;
            li.QuadPart = pos + count;
            LARGE_INTEGER np;
            if (!SetFilePointerEx(out, li, &np, FILE_BEGIN)) goto end;
            if (np.QuadPart != pos + count) goto end;
        } else {
            DWORD wrote = 0;
            if (!WriteFile(out, buf, count, &wrote, NULL)) goto end;
            if (wrote < count) goto end;
        }
        pos += count;
    }

    result = true;
end:
    if (!result) errno = syserror();
    if (in != INVALID_HANDLE_VALUE) CloseHandle(in);
    if (out != INVALID_HANDLE_VALUE) CloseHandle(out);
    if (out != INVALID_HANDLE_VALUE && !result) DeleteFileA(to);
    return result;
}

#else

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

bool mksparse(const char *path, size_t size, unsigned perms) {
    if (!path || !size) return false;
    if (!perms) perms = 0664;
    perms |= 0400U; // always owber writable,,,
    perms &= 0777U; // exclude special perms

    // skip if exists...
    struct stat ino;
    if (stat(path, &ino) == 0) return false;

    int fd = open(path, O_WRONLY | O_CREAT, perms);
    if (fd < 0) return false;

    bool result = true;
    if (ftruncate(fd, (off_t)size) != 0) result = false;
    close(fd);
    return result;
}

bool dupsparse(const char *from, const char *to, uint16_t page) {
    if (!from || !to || !sparse_pagesize(page)) return false;

    struct stat ino;
    bool result = false;
    char buf[page];
    int in = -1, out = -1;

    in = open(from, O_RDONLY);
    if (in < 0) goto end;
    if (fstat(in, &ino) != 0) goto end;
    if (!S_ISREG(ino.st_mode)) goto end;

    if (!mksparse(to, ino.st_size, ino.st_mode & 0777U)) goto end;
    out = open(to, O_WRONLY);
    if (out < 0) goto end;

    off_t pos = 0;
    for (;;) {
        ssize_t count = read(in, buf, sizeof(buf));
        if (count == 0) break;
        if (count < 0) goto end;
        if (is_sparse(buf, count)) {
            off_t r = lseek(out, pos + count, SEEK_SET);
            if (r != pos + count) goto end;
        } else {
            ssize_t wrote = write(out, buf, count);
            if (wrote < count) goto end;
        }
        pos += count;
    }

    result = true;
end:
    close(in);
    close(out);
    if (out > 0 && !result) unlink(to);
    return result;
}

#endif
