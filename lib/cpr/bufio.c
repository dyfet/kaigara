// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#include "bufio.h"
#include "socket.h"
#include "memory.h"

#include <unistd.h>
#ifndef _WIN32
#include <poll.h>
#endif
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>

void cpr_freebuf(bufio_t *r) {
    if (!r) return;
    cpr_flushbuf(r);

#ifndef _WIN32
    if (r->fd > -1 && isatty(r->fd)) {
        tcdrain(r->fd);
        tcsetattr(r->fd, TCSANOW, &r->tty);
        free(r);
        return;
    }
#endif

    if (r->fd > -1) {
#ifdef _WIN32
        if (r->socket) {
            shutdown(r->fd, SD_BOTH);
            closesocket(r->fd);
            r->fd = -1;
        }
#else
        struct stat ino;
        if (!fstat(r->fd, &ino) && S_ISSOCK(ino.st_mode))
            shutdown(r->fd, SHUT_RDWR);
#endif
        if (r->fd > 2)
            close(r->fd);
    }
    free(r);
}

bufio_t *cpr_sockbuf(int so, size_t bufsize) {
#ifdef _WIN32
    bufio_t *b = cpr_makebuf(so, bufsize);
    if (b) b->socket = true;
    return b;
#else
    return cpr_makebuf(so, bufsize);
#endif
}

bufio_t *cpr_makebuf(int fd, size_t bufsize) {
    if (fd < 0 || !bufsize) return NULL;
    bufio_t *r = malloc(sizeof(bufio_t) + (bufsize * 2));
    if (!r) return NULL;
#ifdef _WIN32
    r->socket = false;
#else
    if (isatty(fd)) {
        struct termios t;
        if (tcgetattr(fd, &r->tty) < 0) {
            free(r);
            return NULL;
        }
        tcgetattr(fd, &t);
        t.c_lflag &= ~(ICANON | ECHO | ISIG);
        t.c_iflag &= ~(IXON | ICRNL);
        t.c_oflag &= ~(OPOST);
        t.c_cc[VMIN] = 1;  // read returns after 1 byte
        t.c_cc[VTIME] = 0; // no timeout
        if (tcsetattr(fd, TCSANOW, &t) < 0) {
            free(r);
            return NULL;
        }
    }
#endif

    r->fd = fd;
    r->bufsize = bufsize;
    r->start = r->end = 0;
    return r;
}

bool cpr_resetbuf(bufio_t *r, size_t consume) {
    if (!r || consume > r->bufsize) return false;
    if (r->start + consume <= r->end)
        r->start += consume;
    else
        return false;
    size_t remains = r->end - r->start;
    if (r->start > 0 && remains)
        memmove(r->buf, &r->buf[r->start], remains);
    r->start = 0;
    r->end = remains;
    if (remains < r->bufsize)
        return cpr_fillbuf(r, r->bufsize - remains);
    else
        return true;
}

bool cpr_flushbuf(bufio_t *w) {
    if (!w || !w->put) return false;
    char *out = ((char *)w) + sizeof(bufio_t) + w->bufsize;
    ssize_t result;
#ifdef _WIN32
    if (w->socket) {
        result = send(w->fd, out, w->put, 0);
        goto writer;
    }
#endif
    result = write(w->fd, out, w->put); // FlawFinder: ignore
writer:                                 // NOLINT
    if ((size_t)result < w->put) {
        size_t remaining = w->put - (size_t)result;
        memmove(out, out + result, remaining);
        w->put = remaining;
        return false; // partial flush
    }
    if (result < w->put) return false;
    w->put = 0;
    return true;
}

bool cpr_xputbuf(bufio_t *w, const void *data, size_t request) {
    if (!data || !w || request > w->bufsize) return false;
    char *out = ((char *)w) + sizeof(bufio_t) + w->bufsize;
    if (request + w->put > w->bufsize) {
        if (!cpr_flushbuf(w)) return false;
    }
    cpr_memcpy(&out[w->put], w->bufsize - w->put, data, request);
    w->put += request;
    return true;
}

bool cpr_sputbuf(bufio_t *w, const char *text) {
    if (!text) return false;
    // +1 so if string is too big it falls thru false
    return cpr_xputbuf(w, text, cpr_strlen(text, w->bufsize + 1));
}

bool cpr_fillbuf(bufio_t *r, size_t request) {
    if (!r || request > r->bufsize) return false;
    size_t remains = r->end - r->start;
    size_t avail = r->bufsize - r->start;
    bool refill = false;
    if (request == 0) { // refill for delim parsing scanners
        request = r->bufsize;
        refill = true;
    }
    // if we don't have enough data...
    if (remains < request) { // see if we need to move
        if (avail < request && r->start < r->end) {
            memmove(r->buf, &r->buf[r->start], r->end - r->start);
            r->end -= r->start;
            r->start = 0;
        }
        // FlawFinder: read any extra data to complete request
        ssize_t n;
#ifdef _WIN32
        if (r->socket) {
            n = recv(r->fd, &r->buf[r->end], r->bufsize - r->end, 0);
            goto reader;
        }
#endif
        n = read(r->fd, &r->buf[r->end], r->bufsize - r->end); // FlawFinder: ignore
    reader:                                                    // NOLINT
        if (n > 0) {
            r->end += n;
            r->buf[r->end] = 0;
            if ((r->end - r->start) < request) return false;
        } else {
            r->buf[r->end] = 0;
            if (n == 0) return false; // always false if eof...
            return refill;            // for parser can have less than request
        }
    } else
        r->buf[r->end] = 0; // use null byte, even if full, overflow space
    return true;
}

const char cpr_cgetbuf(bufio_t *r) {
    if (!r) return 0;
    const char *cp = cpr_xgetbuf(r, 1);
    if (!cp) return 0;
    return *cp;
}

bool cpr_cputbuf(bufio_t *r, char ch) {
    if (!r) return false;
    return cpr_xputbuf(r, &ch, 1);
}

const void *cpr_xgetbuf(bufio_t *r, size_t request) {
    if (!r || r->bufsize < request) return NULL;
    if (!cpr_fillbuf(r, request)) return NULL;
    void *out = &r->buf[r->start];
    r->start += request;
    return out;
}

const char *cpr_lgetbuf(bufio_t *r, size_t *outlen, const char *delim) {
    if (!r) return NULL;
    if (!delim) delim = "\n";
    size_t delim_len = cpr_strlen(delim, 16);
    size_t scan = r->start;
    for (;;) {
        while (scan + delim_len <= r->end) {
            if (memcmp(&r->buf[scan], delim, delim_len) == 0) {
                size_t len = scan - r->start;
                const char *result = &r->buf[r->start];
                if (outlen) {
                    *outlen = len;
                } else {
                    if (r->start + len < r->bufsize) {
                        r->buf[r->start + len] = 0;
                    }
                }

                r->start = scan + delim_len;
                return result;
            }
            scan++;
        }
        if (!cpr_fillbuf(r, 0)) // try in partial mode
            return NULL;
    }
}

#ifndef _WIN32
int cpr_waitbuf(const bufio_t *r, int timeout_ms) {
    if (!r || r->fd < 0) return -1;
    struct pollfd pfd = {
    .fd = r->fd,
    .events = POLLIN};

    while (1) {
        int rv = poll(&pfd, 1, timeout_ms);
        if (rv > 0) {
            if (pfd.revents & POLLIN) return 1; // ready to read
            if (pfd.revents & (POLLERR | POLLHUP | POLLNVAL)) return -2;
            return -3; // unexpected state
        } else if (rv == 0) {
            return 0; // timeout
        } else if (errno == EINTR) {
            continue; // retry
        } else {
            return -4; // poll error
        }
    }
}
#endif

bool cpr_fmtbuf(bufio_t *w, size_t estimated, const char *fmt, ...) {
    if (!w || !fmt || !estimated || estimated > w->bufsize) return false;
    char *out = (char *)w + sizeof(bufio_t) + w->bufsize;
    if (w->put + estimated > w->bufsize) {
        if (!cpr_flushbuf(w)) return false;
    }

    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(&out[w->put], estimated, fmt, ap); // FlawFinder: checked
    va_end(ap);

    if (n < 0 || (size_t)n >= estimated) return false;
    w->put += (size_t)n;
    return true;
}
