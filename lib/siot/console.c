// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#include "console.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static bool wrap = true;

#ifdef _WIN32
#define MICROSOFT_WINDOWS_WINBASE_H_DEFINE_INTERLOCKED_CPLUSPLUS_OVERLOADS 0
#include <windows.h>

static bool is_tty(HANDLE handle) {
    if (handle == INVALID_HANDLE_VALUE) return false;
    DWORD type = GetFileType(handle);
    return type == FILE_TYPE_CHAR;
}

bool sio_startup() {
    HANDLE in = GetStdHandle(STD_INPUT_HANDLE);
    if (in == INVALID_HANDLE_VALUE) return false;
    if (!is_tty(in)) return false;
    return true;
}

bool sio_passwd(char *buf, size_t size, const char *prompt) {
    if (prompt == NULL) prompt = "Password: ";
    if (!buf || size == 0) return false;
    DWORD mode;
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    if (hStdin == INVALID_HANDLE_VALUE || !GetConsoleMode(hStdin, &mode))
        return false;

    fputs(prompt, stdout);
    fflush(stdout);
    if (!SetConsoleMode(hStdin, mode & ~(ENABLE_ECHO_INPUT))) return false;
    if (!fgets(buf, (int)size, stdin)) {
        SetConsoleMode(hStdin, mode);
        return false;
    }

    SetConsoleMode(hStdin, mode);
    fputs("\n", stdout);
    buf[strcspn(buf, "\r\n")] = '\0';
    return true;
}

#else

#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <poll.h>

bool sio_startup() {
    if (!isatty(1)) return false;
    return true;
}

#include <termios.h>
bool sio_passwd(char *buf, size_t size, const char *prompt) {
    if (prompt == NULL) prompt = "Password: ";
    if (!buf || size == 0) return false;
    struct termios old, new;
    if (tcgetattr(0, &old) != 0) return false;
    new = old;
    new.c_lflag &= ~ECHO;
    if (tcsetattr(0, TCSAFLUSH, &new) != 0) return false;

    fputs(prompt, stdout);
    fflush(stdout);
    if (!fgets(buf, (int)size, stdin)) buf[0] = '\0';
    tcsetattr(0, TCSAFLUSH, &old);
    printf("\n");
    size_t len = strlen(buf); // FlawFinder: ignore
    if (len > 0 && buf[len - 1] == '\n') buf[len - 1] = '\0';
    return true;
}

#endif

void sio_cls(void) {
    fputs("\033[2J\033[H", stdout);
    fflush(stdout);
}

void sio_reset(void) {
    fputs("\033[0m", stdout);
    fflush(stdout);
}

bool sio_wrap(bool enable) {
    bool prior = wrap;
    if (prior == enable) return enable;
    wrap = enable;
    if (enable)
        fputs("\033[?7h", stdout);
    else
        fputs("\033[?7l", stdout);
    fflush(stdout);
    return prior;
}
