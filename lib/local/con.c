// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 202y David Sugar <tychosoft@gmail.com>

#include "con.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <stdarg.h>
#include <ctype.h>

static bool wrap = true;
static bool cls = false;
static con_break_t handler = NULL;
static int lines = 0, cols = 0;
static int key_mode = KEY_INPUT_DISABLED;

#ifdef _WIN32
#define MICROSOFT_WINDOWS_WINBASE_H_DEFINE_INTERLOCKED_CPLUSPLUS_OVERLOADS 0
#include <windows.h>

static DWORD active = 0, restore = 0;

static BOOL CtrlHandler(DWORD event) {
    switch (event) {
    case CTRL_C_EVENT:
    case CTRL_BREAK_EVENT:
        printf("^C");
        if (handler) handler();
        return TRUE;
    default:
        if (handler) handler();
        return TRUE;
    }
}

static bool is_tty(HANDLE handle) {
    if (handle == INVALID_HANDLE_VALUE) return false;
    DWORD type = GetFileType(handle);
    return type == FILE_TYPE_CHAR;
}

static int inkey(int timeout_ms, KEY_EVENT_RECORD *out) {
    HANDLE in = GetStdHandle(STD_INPUT_HANDLE);
    if (in == INVALID_HANDLE_VALUE) return EOF;
    DWORD start = GetTickCount();
    INPUT_RECORD rec;
    DWORD nread;
    for (;;) {
        DWORD count = 0;
        if (!GetNumberOfConsoleInputEvents(in, &count)) return EOF;
        if (count > 0) {
            if (!ReadConsoleInputW(in, &rec, 1, &nread)) return EOF;
            if (rec.EventType == KEY_EVENT && rec.Event.KeyEvent.bKeyDown) {
                KEY_EVENT_RECORD *k = &rec.Event.KeyEvent;
                WORD vk = k->wVirtualKeyCode;
                WCHAR ch = k->uChar.UnicodeChar;
                switch (vk) {
                case VK_SHIFT:
                case VK_LSHIFT:
                case VK_RSHIFT:
                case VK_CONTROL:
                case VK_LCONTROL:
                case VK_RCONTROL:
                case VK_MENU:
                case VK_LMENU:
                case VK_RMENU:
                    continue;
                }

                if (ch != 0) {
                    if (out) *out = *k;
                    return (int)ch;
                }
                if (out) *out = *k;
                return -(int)vk;
            }
            continue;
        }

        DWORD now = GetTickCount();
        if ((int)(now - start) >= timeout_ms) return 0;
        Sleep(10);
    }
}

bool con_istty() {
    return is_tty(GetStdHandle(STD_INPUT_HANDLE));
}

bool con_refesh() {
    return false;
}

bool con_break(con_break_t func) {
    HANDLE in = GetStdHandle(STD_INPUT_HANDLE);
    if (in == INVALID_HANDLE_VALUE) return false;
    if (!is_tty(in)) return false;
    if (!func) SetConsoleCtrlHandler(CtrlHandler, FALSE);
    handler = func;
    if (func) SetConsoleCtrlHandler(CtrlHandler, TRUE);
    return true;
}

bool con_startup(int key_input) {
    key_mode = key_input;
    HANDLE in = GetStdHandle(STD_INPUT_HANDLE);
    if (in == INVALID_HANDLE_VALUE) return false;
    if (!is_tty(in)) return false;
    if (key_input) {
        GetConsoleMode(in, &restore);
        active = restore;
        active &= ~(ENABLE_LINE_INPUT |
                    ENABLE_ECHO_INPUT |
                    ENABLE_PROCESSED_INPUT |
                    ENABLE_WINDOW_INPUT |
                    ENABLE_MOUSE_INPUT);
        SetConsoleMode(in, active);
        FlushConsoleInputBuffer(in);
    }
    return true;
}

void con_shutdown() {
    if (key_mode) {
        HANDLE in = GetStdHandle(STD_INPUT_HANDLE);
        SetConsoleMode(in, restore);
        key_mode = KEY_INPUT_DISABLED;
        con_wrap(true);
    }

    con_reset();
    if (cls) {
        con_goto(lines, 1);
        con_eol();
        cls = false;
    }
}

bool con_passwd(char *buf, size_t size, const char *prompt) {
    if (prompt == NULL) prompt = "Password: ";
    if (!buf || size == 0) return false;
    DWORD mode;
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    if (hStdin == INVALID_HANDLE_VALUE || !GetConsoleMode(hStdin, &mode))
        return false;

    fputs(prompt, stdout);
    fflush(stdout);
    if (!SetConsoleMode(hStdin, mode & ~(ENABLE_ECHO_INPUT))) return false; // BOLINT
    if (!fgets(buf, (int)size, stdin)) {
        SetConsoleMode(hStdin, mode);
        return false;
    }

    SetConsoleMode(hStdin, mode);
    fputs("\n", stdout);
    buf[strcspn(buf, "\r\n")] = '\0';
    return true;
}

int con_inkey(int timeout) {
    if (!key_mode) return EOF;
    return inkey(timeout, NULL);
}

#else

#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <errno.h>

static volatile bool winch = false;
static struct termios active, restore;

static void resize() {
#ifdef TIOCGWINSZ
    struct winsize ws;
    if (ioctl(1, TIOCGWINSZ, &ws) == 0) {
        lines = ws.ws_row;
        cols = ws.ws_col;
    }
#endif
}

static void sigfunc(int signo) {
    (void)signo;
    if (handler) handler();
}

static void sigwinch(int signo) {
    (void)signo;
    winch = true;
}

bool con_refresh() {
    if (!winch) return false;
    winch = false;
    resize();
    return true;
}

bool con_istty() {
    return isatty(0) == 1;
}

bool con_break(con_break_t func) {
    if (!con_istty()) return false;
    if (!func) signal(SIGINT, SIG_DFL);
    handler = func;
    if (func) signal(SIGINT, sigfunc);
    return true;
}

bool con_startup(int key_input) {
    key_mode = key_input;
    if (!con_istty()) return false;
    signal(SIGWINCH, sigwinch);
    resize();
    if (key_input) {
        tcgetattr(0, &active);
        tcgetattr(0, &restore);
        active.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON | IXANY); // NOLINT
        active.c_oflag &= ~OPOST;                                            // NOLINT
        active.c_cflag |= CS8;                                               // NOLINT
        active.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);                  // NOLINT
        active.c_cc[VMIN] = 1;
        active.c_cc[VTIME] = 0;
        tcsetattr(0, TCSANOW, &active);
    }
    return true;
}

void con_shutdown() {
    if (key_mode) {
        tcsetattr(0, TCSANOW, &restore);
        key_mode = KEY_INPUT_DISABLED;
        con_wrap(true);
    }

    con_reset();
    if (cls) {
        con_goto(lines, 1);
        con_eol();
    }
}

bool con_passwd(char *buf, size_t size, const char *prompt) {
    if (prompt == NULL) prompt = "Password: ";
    if (!buf || size == 0) return false;
    struct termios old, new;
    if (tcgetattr(0, &old) != 0) return false;
    new = old;
    new.c_lflag &= ~ECHO; // NOLINT
    if (tcsetattr(0, TCSAFLUSH, &new) != 0) return false;

    fputs(prompt, stdout);
    fflush(stdout);
    if (!fgets(buf, (int)size, stdin)) buf[0] = '\0';
    tcsetattr(0, TCSAFLUSH, &old);
    printf("\n");
    size_t len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n') buf[len - 1] = '\0';
    return true;
}

int con_inkey(int timeout) {
    struct pollfd pfd;
    int rc;
retry:
    pfd.fd = 0;
    pfd.events = POLLIN;
    pfd.revents = 0;
    rc = poll(&pfd, 1, timeout);
    if (rc < 0 && errno == EINTR) goto retry;
    if (rc < 1) return 0;

    char ch = 0;
    ssize_t len = read(0, &ch, 1);
    if (len < 1) return EOF;
    return ch;
}

#endif

int con_cols() { return cols; }
int con_lines() { return lines; }

void con_cls() {
    fputs("\033[2J\033[H", stdout);
    fflush(stdout);
    cls = true;
}

void con_eol() {
    fputs("\033[K", stdout);
    fflush(stdout);
}

void con_reset() {
    fputs("\033[0m\033[?12h\033[?25h\033[0q", stdout);
    fflush(stdout);
}

bool con_wrap(bool enable) {
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

bool con_goto(int line, int col) {
    if (!lines) return false;
    if (line < 0) line = lines + line + 1;
    if (col < 0) col = cols + col + 1;
    if (!line && col > 9 && col <= cols) {
        printf("\033[%dG", col);
        fflush(stdout);
        return true;
    }

    if (!line || !col) return false;
    if (line < 1 || line > lines) return false;
    if (col < 1 || col > cols) return false;
    printf("\033[%d;%dH", line, col);
    fflush(stdout);
    return true;
}

size_t con_ignore() {
    size_t count = 0;
    for (;;) {
        int key = con_inkey(key_mode);
        if (key == EOF) return EOF;
        if (!key) break;
        ++count;
    }
    return count;
}

size_t con_escape(char *buf, size_t len) {
    if (!buf || len < 2) return con_ignore();
    size_t pos = 1;
    buf[0] = 033; // ESC
    while (pos < (len - 1)) {
        int key = con_inkey(key_mode);
        if (!key || key == EOF) break;
        buf[pos++] = (char)key;
        if (pos == 2) continue;
        if (key >= 0x40 && key <= 0x7e) break;
    }
    buf[pos] = 0;
    return pos;
}

bool con_attrstr(char *buf, size_t size, int reset, ...) {
    if (!buf || size < 8) return false;
    int pos;
    bool sep = false;
    if (reset) {
        snprintf(buf, size, "\033[0");
        pos = 3;
        sep = true;
    } else {
        snprintf(buf, size, "\033[");
        pos = 2;
    }
    bool rc = true;
    va_list attrs;
    va_start(attrs, reset);
    for (;;) {
        if (pos > (size - 5)) {
            rc = false;
            break;
        }
        int attr = va_arg(attrs, int);
        if (!attr) break;
        if (sep) buf[pos++] = ';';
        sep = true;
        snprintf(buf + pos, 4, "%d", attr);
        if (attr > 99) ++pos;
        if (attr > 9) ++pos;
        ++pos;
    }
    va_end(attrs);
    buf[pos++] = 'm';
    buf[pos] = 0;
    return rc;
}

bool con_left(int line, int col, int size, const char *text, const char *pre) {
    if (!text || !con_goto(line, col)) return false;
    if (pre) fputs(pre, stdout);
    unsigned pos = 0;
    while (pos < size && text[pos])
        fputc(text[pos++], stdout);
    while (pos < size) {
        ++pos;
        fputc(' ', stdout);
    }
    fflush(stdout);
    return true;
}

bool con_right(int line, int col, int size, const char *text, const char *pre) {
    if (!text || !con_goto(line, col)) return false;
    if (pre) fputs(pre, stdout);
    size_t len = strlen(text);
    if (len > size) len = size;
    size_t off = size - len;
    unsigned pos = 0;
    while (off > 0) {
        fputc(' ', stdout);
        --off;
    }
    while (pos < size && text[pos])
        fputc(text[pos++], stdout);
    fflush(stdout);
    return true;
}

bool con_center(int line, int col, int size, const char *text, const char *pre) {
    if (!text || !con_goto(line, col)) return false;
    if (pre) fputs(pre, stdout);
    size_t len = strlen(text);
    if (len > size) len = size;
    size_t off = (size - len) / 2;
    size_t tail = 0;
    if (off) tail = size - off - len;
    unsigned pos = 0;
    while (off > 0) {
        fputc(' ', stdout);
        --off;
    }
    while (pos < size && text[pos])
        fputc(text[pos++], stdout);
    while (tail > 0) {
        fputc(' ', stdout);
        --tail;
    }
    fflush(stdout);
    return true;
}

bool con_input(const char *prompt, char *buf, size_t size, bool nl) {
    if (!key_mode || !buf || size < 2) return false;
    if (prompt) fputs(prompt, stdout);
    fflush(stdout);
    --size;
    int pos = 0;
    for (;;) {
        int key = con_inkey(-1);
        if (!key) continue;
        if (key == -1) {
            buf[0] = 0;
            return false;
        }
        if (key == 3) {
            printf("^C");
            if (nl)
                con_nl();
            else
                fflush(stdout);
            buf[0] = 0;
            return false;
        }
        if (key == 27) {
            con_ignore();
            pos = 0;
            break;
        }
        if (key == 0x7f) key = 8;
        if (key == 8 && !pos) {
            fputc(7, stdout);
            fflush(stdout);
            continue;
        }
        if (key == 8) {
            printf("\b \b");
            fflush(stdout);
            --pos;
            continue;
        }
        if (key == 13 || key == 10) break;
        if (key < 0) continue;
        if (!isalnum(key) && !ispunct(key) && key != ' ') continue;
        if (pos >= size) {
            fputc(7, stdout);
            fflush(stdout);
            continue;
        }
        buf[pos++] = (char)key;
        printf("%c", (char)key);
        fflush(stdout);
    }
    buf[pos] = 0;
    if (nl) con_nl();
    return true;
}

int con_choose(const char *prompt, const char *choices, int timeout, bool nl) {
    if (!key_mode) return -1;
    if (prompt) fputs(prompt, stdout);
    fflush(stdout);
    int key;
    for (;;) {
        key = con_inkey(timeout);
        if (key == -1 || key == 0) break;
        if (key == 27) {
            con_ignore();
            key = 0;
            break;
        }
        if (key == 3) {
            key = -1;
            printf("^C");
            break;
        }
        if (key < 1 || key > 255) continue;
        char code = (char)(tolower(key));
        if (strchr(choices, code)) {
            printf("%c", code);
            if (nl) con_nl();
            fflush(stdout);
            return code;
        }
    }
    if (nl)
        con_nl();
    else
        fflush(stdout);
    return key;
}

void con_nl() {
    printf("\r\n");
    fflush(stdout);
}
