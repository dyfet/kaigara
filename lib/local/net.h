// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 David Sugar <tychosoft@gmail.com>

#ifndef LOCAL_NET_H
#define LOCAL_NET_H

#include <cpr/address.h>
#include <cpr/thread.h>
#include <stdio.h>

#ifdef _WIN32
#define IFACE_SIZE 512
#else
#define IFACE_SIZE 16
#endif

typedef struct {
    int so;
    cpr_semaphore_t *sem;
    FILE *log;
    thrd_start_t start;
    const char *tag;
} net_accept_t;

typedef struct {
    int so;
    cpr_semaphore_t *sem;
    char *origin;
} net_session_t;

bool net_iface(int family, const char *iface, uint16_t port, sockaddr_t *store);
int net_listen(const struct sockaddr *addr, int backlog);
int net_recvline(int so, char *buf, size_t size, int timeout);
int net_readline(int so, char *buf, size_t size, int timeout);
int net_sendtext(int so, const char *text, size_t size);
int net_wait(int so, int timeout);
int net_session(int so, bool live);
int net_proxy(const char *host, uint16_t port, const char *ident, const struct sockaddr *addr);
int net_connect(const struct sockaddr *addr);
int net_sendtext(int so, const char *text, size_t size);
bool net_accept(thrd_t *thread, net_accept_t *server);
bool net_strmake(char **str, struct sockaddr_storage *store);
void net_strfree(char **str);
void net_shutdown();
bool net_host(int family, int type, const char *host, uint16_t port, sockaddr_t *store);
bool net_service(int family, int type, const char *host, const char *service, uint16_t port, sockaddr_t *store);
bool net_getiface(const sockaddr_t *addr, char *buf, size_t size);
size_t net_sendfd(int so, int fd, size_t count, size_t chunk);
size_t net_recvfd(int so, int fd, size_t count, size_t chunk);
__attribute__((format(printf, 3, 4))) int net_printf(int so, size_t size, const char *fmt, ...);

#endif
