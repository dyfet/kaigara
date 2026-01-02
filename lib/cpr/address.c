// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#include "address.h"
#include "strchar.h"
#include "memory.h"

bool cpr_addport(sockaddr_t *addr, uint16_t port) {
    if (!addr) return false;
    if (addr->ss_family == AF_INET) {
        struct sockaddr_in *a4 = (struct sockaddr_in *)addr;
        a4->sin_port = htons(port);
        return true;
    }
    if (addr->ss_family == AF_INET) {
        struct sockaddr_in6 *a4 = (struct sockaddr_in6 *)addr;
        a4->sin6_port = htons(port);
        return true;
    }
    return false;
}

int cpr_setaddr(sockaddr_t *store, const char *addr, uint16_t port) {
    if (!store) return AF_UNSPEC;
    cpr_memset(store, 0, sizeof(sockaddr_t));
    if (!addr || !*addr) return AF_UNSPEC;
    if (strchr(addr, '.')) {
        struct sockaddr_in *a4 = (struct sockaddr_in *)store;
        if (inet_pton(AF_INET, addr, &a4->sin_addr) == 1) {
            store->ss_family = AF_INET;
            a4->sin_port = htons(port);
        }
    } else if (strchr(addr, ':')) {
        struct sockaddr_in6 *a6 = (struct sockaddr_in6 *)store;
        if (inet_pton(AF_INET6, addr, &a6->sin6_addr) == 1) {
            store->ss_family = AF_INET6;
            a6->sin6_port = htons(port);
        }
    }
    return store->ss_family;
}

char *cpr_getaddr(sockaddr_t *store) {
    char buf[INET6_ADDRSTRLEN + 1];
    if (!store) return NULL;
    buf[0] = 0;
    if (store->ss_family == AF_UNSPEC)
        return cpr_strdup("*", 2);

    const char *out = NULL;
    if (store->ss_family == AF_INET) {
        struct sockaddr_in *a4 = (struct sockaddr_in *)store;
        out = inet_ntop(AF_INET, &a4->sin_addr, buf, sizeof(buf));
    } else if (store->ss_family == AF_INET6) {
        struct sockaddr_in6 *a6 = (struct sockaddr_in6 *)store;
        out = inet_ntop(AF_INET6, &a6->sin6_addr, buf, sizeof(buf));
    }
    if (out) return cpr_strdup(out, sizeof(buf));
    return NULL;
}

bool cpr_getbind(const char *to, int family, sockaddr_t *store, uint16_t port) {
    cpr_memset(store, 0, sizeof(struct sockaddr_storage));
    if ((family != AF_INET && family != AF_INET6) || !to || *to == 0) return false;
    if (!strcmp(to, "*")) {
        store->ss_family = family;
        cpr_addport(store, port);
        return true;
    }
    if ((family == AF_INET) && strchr(to, '.')) {
        store->ss_family = AF_INET;
        cpr_addport(store, port);
        struct sockaddr_in *a4 = (struct sockaddr_in *)store;
        if (inet_pton(AF_INET, to, &a4->sin_addr) == 1) return true;
    } else if ((family == AF_INET6) && strchr(to, ':')) {
        store->ss_family = AF_INET6;
        cpr_addport(store, port);
        struct sockaddr_in6 *a6 = (struct sockaddr_in6 *)store;
        if (inet_pton(AF_INET6, to, &a6->sin6_addr) == 1) return true;
    }

#ifdef _WIN32
    ULONG bufsize = 8192;
    PIP_ADAPTER_ADDRESSES list = (PIP_ADAPTER_ADDRESSES)(malloc(bufsize));
    if (!list) return false;

    WORD result = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, list, &bufsize);
    if (result == ERROR_BUFFER_OVERFLOW) {
        free(list);
        list = (PIP_ADAPTER_ADDRESSES)(malloc(bufsize));
        result = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, list, &bufsize);
    };

    if (result != NO_ERROR) {
        free(list);
        return false;
    }

    const struct sockaddr *target = NULL;
    PIP_ADAPTER_ADDRESSES entry;
    bool found = false;
    for (entry = list; !found && entry != NULL; entry = entry->Next) {
        if (!entry->AdapterName || !eq(to, entry->AdapterName)) continue;
        PIP_ADAPTER_UNICAST_ADDRESS unicast;
        for (unicast = entry->FirstUnicastAddress; !found && unicast != NULL; unicast = unicast->Next) {
            if (unicast->Address.lpSockaddr && unicast->Address.lpSockaddr->sa_family == family) {
                target = unicast->Address.lpSockaddr;
                found = true;
                break;
            }
        }
        if (found) break;
    }

    if (target) {
        cpr_memcpy(store, sizeof(sockaddr_t), target, cpr_socklen(target));
        cpr_addport(store, port);
    }

    free(list);
    return target != NULL;
#else
    const struct sockaddr *addr = NULL;
    iface_t list = NULL, node = NULL;
    if (getifaddrs(&list)) return false;
    node = list;
    while (!addr && node) {
        if (node->ifa_addr && node->ifa_addr->sa_family == family && eq(node->ifa_name, to))
            addr = node->ifa_addr;
        node = node->ifa_next;
    }
    if (addr != NULL) {
        cpr_memcpy(store, sizeof(struct sockaddr_storage), addr, cpr_socklen(addr));
        cpr_addport(store, port);
    }
    if (list) freeifaddrs(list);
    return addr != NULL;
#endif
}

bool cpr_gethost(const char *host, const char *service, int family, int type, sockaddr_t *store) {
    if (!host) return false;
    if (family != AF_INET && family != AF_INET6) return false;
    struct addrinfo hints, *res = NULL;
    cpr_memset(&hints, 0, sizeof(hints));
    hints.ai_family = family;
    hints.ai_socktype = type;
    hints.ai_flags = AI_ADDRCONFIG;

    const int rc = getaddrinfo(host, service, &hints, &res);
    if (rc != 0 || !res) return false;

    cpr_memcpy(store, sizeof(sockaddr_t), res->ai_addr, res->ai_addrlen);
    freeaddrinfo(res);
    return true;
}
