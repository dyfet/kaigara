// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#ifndef CPR_SOCKET_H
#define CPR_SOCKET_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#include "system.h"
typedef int socklen_t;
#else
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <poll.h>
#include <ifaddrs.h>
#include <stdbool.h>

typedef struct ifaddrs *iface_t;
#endif

#include <sys/types.h>
#include <fcntl.h>

// socket casting types

socklen_t cpr_socklen(const struct sockaddr *addr);
void cpr_sockclose(int so);
int cpr_sockinit();
int cpr_getch(int so);
bool cpr_putch(int so, char code);
ssize_t cpr_getline(int so, char *text, size_t max, bool echo);
ssize_t cpr_putline(int so, const char *str, size_t max);

inline static struct sockaddr *to_sockaddr(struct sockaddr_storage *storage) {
    return (struct sockaddr *)storage;
}

inline static const struct sockaddr_un *to_unix(const struct sockaddr *addr) {
    return (struct sockaddr_un *)addr;
}

inline static const struct sockaddr_in *to_in4(const struct sockaddr *addr) {
    return (struct sockaddr_in *)addr;
}

inline static const struct sockaddr_in6 *to_in6(const struct sockaddr *addr) {
    return (struct sockaddr_in6 *)addr;
}

#ifdef __cplusplus
}
#endif
#endif
