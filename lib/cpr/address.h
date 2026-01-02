// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#ifndef CPR_ADDRESS_H
#define CPR_ADDRESS_H

#include "socket.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sockaddr_storage sockaddr_t;

int cpr_setaddr(sockaddr_t *store, const char *addr, uint16_t port);
char *cpr_getaddr(sockaddr_t *store);
bool cpr_addport(sockaddr_t *store, uint16_t port);
bool cpr_getbind(const char *to, int family, sockaddr_t *store, uint16_t port);
bool cpr_gethost(const char *host, const char *service, int family, int type, sockaddr_t *store);

#ifdef __cplusplus
}
#endif
#endif
