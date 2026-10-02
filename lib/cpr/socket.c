// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#include "socket.h"
#include "strchar.h"

#include <unistd.h>
#include <errno.h>

#ifdef _WIN32
FILE *cpr_sockopen(int so, const char *mode) {
    int fd = _open_osfhandle((intptr_t)so, 0);
    if (fd == -1) return NULL;
    FILE *f = _fdopen(fd, mode);
    if (!f) _close(fd);
    return f;
}

int cpr_sockerror() {
    int wsaErr = WSAGetLastError();
    switch (wsaErr) {
    case WSAEINTR:
        return EINTR;
    case WSAEBADF:
        return EBADF;
    case WSAEACCES:
        return EACCES;
    case WSAEFAULT:
        return EFAULT;
    case WSAEINVAL:
        return EINVAL;
    case WSAEMFILE:
        return EMFILE;
    case WSAEWOULDBLOCK:
        return EWOULDBLOCK; // or EAGAIN
    case WSAEINPROGRESS:
        return EINPROGRESS;
    case WSAEALREADY:
        return EALREADY;
    case WSAENOTSOCK:
        return ENOTSOCK;
    case WSAEDESTADDRREQ:
        return EDESTADDRREQ;
    case WSAEMSGSIZE:
        return EMSGSIZE;
    case WSAEPROTOTYPE:
        return EPROTOTYPE;
    case WSAENOPROTOOPT:
        return ENOPROTOOPT;
    case WSAEPROTONOSUPPORT:
        return EPROTONOSUPPORT;
    case WSAESOCKTNOSUPPORT:
        return ENOTSUP;
    case WSAEOPNOTSUPP:
        return EOPNOTSUPP;
    case WSAEPFNOSUPPORT:
        return ENOTSUP;
    case WSAEAFNOSUPPORT:
        return EAFNOSUPPORT;
    case WSAEADDRINUSE:
        return EADDRINUSE;
    case WSAEADDRNOTAVAIL:
        return EADDRNOTAVAIL;
    case WSAENETDOWN:
        return ENETDOWN;
    case WSAENETUNREACH:
        return ENETUNREACH;
    case WSAENETRESET:
        return ENETRESET;
    case WSAECONNABORTED:
        return ECONNABORTED;
    case WSAECONNRESET:
        return ECONNRESET;
    case WSAENOBUFS:
        return ENOBUFS;
    case WSAEISCONN:
        return EISCONN;
    case WSAENOTCONN:
        return ENOTCONN;
    case WSAESHUTDOWN:
        return ENOTCONN;
    case WSAETOOMANYREFS:
        return ENOTSUP;
    case WSAETIMEDOUT:
        return ETIMEDOUT;
    case WSAECONNREFUSED:
        return ECONNREFUSED;
    case WSAEHOSTDOWN:
        return EHOSTUNREACH;
    case WSAEHOSTUNREACH:
        return EHOSTUNREACH;
    default:
        return EINVAL; // fallback
    }
}

void cpr_sockdown(int so) {
    shutdown(so, SD_BOTH);
}

void cpr_sockclose(int so) {
    shutdown(so, SD_BOTH);
    closesocket(so);
}

int cpr_sockpoll(struct pollfd *pfd, int count, int timeout) {
    return WSAPoll(pfd, count, timeout);
}

int cpr_sockinit() {
    WSADATA wsa;
    return WSAStartup(MAKEWORD(2, 2), &wsa);
}
#else
FILE *cpr_sockopen(int so, const char *mode) {
    return fdopen(so, mode);
}

int cpr_sockerror() {
    return errno;
}

void cpr_sockdown(int so) {
    shutdown(so, SHUT_RDWR);
}

void cpr_sockclose(int so) {
    shutdown(so, SHUT_RDWR);
    close(so);
}

int cpr_sockpoll(struct pollfd *pfd, int count, int timeout) {
    return poll(pfd, count, timeout);
}

int cpr_sockinit() {
    return 0;
}
#endif

socklen_t cpr_socklen(const struct sockaddr *addr) {
    if (!addr) return 0;
    switch (addr->sa_family) {
    case AF_INET:
        return sizeof(struct sockaddr_in);
    case AF_INET6:
        return sizeof(struct sockaddr_in6);
    default:
        return 0;
    }
}

int cpr_getch(int so) {
    char buf;
    if (recv(so, &buf, 1, 0) == 1) return buf;
    return -1;
}

bool cpr_putch(int so, char code) {
    if (send(so, &code, 1, 0) == 1) return true;
    return false;
}

ssize_t cpr_putline(int so, const char *text, size_t max) {
    if (max) max = 256;
    size_t size = cpr_strlen(text, max);
    if (!size) return 0;
    return send(so, text, size, 0);
}

ssize_t cpr_getline(int so, char *text, size_t max, bool echo) {
    if (!text || max < 2) return 0;
    size_t pos = 0;
    bool cr = false;
    text[0] = 0;
    while (pos < (max - 1)) {
        int code = cpr_getch(so);
        if (code == -1) return -1;
        if (code > 31 && code < 127) {
            cr = false;
            text[pos++] = (char)code;
            text[pos] = 0;
            if (echo) cpr_putch(so, (char)code);
            continue;
        }

        if (code == '\n') {
            if (echo && cr) cpr_putch(so, '\r');
            cpr_putch(so, '\n');
            break;
        }

        if (code == 13 && cr) break;
        if (code == 13) {
            cr = true;
            continue;
        }

        cr = false;
        if (code == 8 || code == 127) { // simple backspace editing...
            if (!pos) continue;
            text[--pos] = 0;
            if (echo) cpr_putline(so, "\b \b", 0);
            continue;
        }
    }
    return (ssize_t)pos;
}
