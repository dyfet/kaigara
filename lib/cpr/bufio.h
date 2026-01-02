// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#ifndef CPR_BUFIO_H
#define CPR_BUFIO_H

#include "strchar.h"

#ifndef _WIN32
#include <termios.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int fd;
#ifdef _WIN32
    bool socket;
#else
    struct termios tty;
#endif
    size_t bufsize;
    size_t start;
    size_t end;
    size_t put;
    char buf[2]; // extra byte to zero end of buf in fetch
} bufio_t;

bufio_t *cpr_sockbuf(int so, size_t bufsize);
bufio_t *cpr_makebuf(int fd, size_t bufsize);
const char *cpr_lgetbuf(bufio_t *r, size_t *outlen, const char *delim);
const void *cpr_xgetbuf(bufio_t *r, size_t request);
const char cpr_cgetbuf(bufio_t *r);
bool clr_resetbuf(bufio_t *r, size_t consume);
bool cpr_fillbuf(bufio_t *r, size_t request);
bool cpr_flushbuf(bufio_t *w);
bool cpr_cputbuf(bufio_t *w, char ch);
bool cpr_xputbuf(bufio_t *w, const void *data, size_t request);
bool cpr_sputbuf(bufio_t *w, const char *text);
bool cpr_fmtbuf(bufio_t *w, size_t estimated, const char *fmt, ...);
int cpr_waitbuf(const bufio_t *r, int timeout_ms);
void cpr_freebuf(bufio_t *r);

#ifdef __cplusplus
}
#endif
#endif
