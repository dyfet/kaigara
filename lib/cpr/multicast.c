// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#include "multicast.h"
#include "strchar.h"
#include "memory.h"

#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

multicast_t if_multicast;

#ifdef _WIN32
int make_multicast(const char *mcast, int family, uint16_t port) {
    int sock = -1;
    cpr_memset(&if_multicast, 0, sizeof(if_multicast));
    ULONG bufsize = 8192;
    PIP_ADAPTER_ADDRESSES list = (PIP_ADAPTER_ADDRESSES)(malloc(bufsize));
    if (!list) {
        perror("pop malloc");
        exit(-1);
    }

    WORD result = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, list, &bufsize);
    if (result == ERROR_BUFFER_OVERFLOW) {
        free(list);
        list = (PIP_ADAPTER_ADDRESSES)(malloc(bufsize));
        result = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, list, &bufsize);
    };

    if (result != NO_ERROR) {
        fprintf(stderr, "*** %d: failed to get pip", (int)result);
        exit(-1);
    }

    const struct sockaddr_in *target = NULL;
    PIP_ADAPTER_ADDRESSES entry;
    bool found = false;
    for (entry = list; !found && entry != NULL; entry = entry->Next) {
        if (!entry->AdapterName || !eq(mcast, entry->AdapterName)) continue;
        if (entry->Flags & IP_ADAPTER_NO_MULTICAST) continue;
        PIP_ADAPTER_UNICAST_ADDRESS unicast;
        for (unicast = entry->FirstUnicastAddress; !found && unicast != NULL; unicast = unicast->Next) {
            if (unicast->Address.lpSockaddr && unicast->Address.lpSockaddr->sa_family == family) {
                if (family == AF_INET)
                    target = (struct sockaddr_in *)&unicast->Address.lpSockaddr;
                found = true;
                break;
            }
        }
        if (found) break;
    }

    if (!entry) {
        fprintf(stderr, "%s: iface not found\b", mcast);
        exit(-2);
    }

    sock = socket(family, SOCK_DGRAM, 0);
    if (sock < 0) {
        fprintf(stderr, "unable to make socket\n");
        exit(-3);
    }

    int reuse = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void *)&reuse, sizeof(reuse)) < 0) {
        fprintf(stderr, "unable to reuse socket\n");
        exit(-4);
    }

    if (family == AF_INET) {
        struct sockaddr_in mcaddr;
        cpr_memset(&mcaddr, 0, sizeof(mcaddr));
        mcaddr.sin_family = AF_INET;
        mcaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        mcaddr.sin_port = htons(port);
        if (bind(sock, (struct sockaddr *)&mcaddr, sizeof(mcaddr))) {
            fprintf(stderr, "unable to BIND socket\n");
            exit(-5);
        }

        struct in_addr if_addr = target->sin_addr;
        if_multicast.ipv4.imr_interface = if_addr;
        if (setsockopt(sock, IPPROTO_IP, IP_MULTICAST_IF, (void *)&if_addr, sizeof(if_addr))) {
            fprintf(stderr, "unable to set IP_MULTICAST_IF\n");
            exit(-6);
        }
    } else if (family == AF_INET6) {
        struct sockaddr_in6 mcaddr6;
        cpr_memset(&mcaddr6, 0, sizeof(mcaddr6));
        mcaddr6.sin6_family = AF_INET6;
        mcaddr6.sin6_addr = in6addr_any; // wildcard bind
        mcaddr6.sin6_port = htons(port);
        if (bind(sock, (struct sockaddr *)&mcaddr6, sizeof(mcaddr6)) < 0) {
            fprintf(stderr, "unable to BIND IPv6 socket\n");
            exit(-5);
        }

        // Scope outbound multicast to the selected interface
        unsigned if_index = if_nametoindex(mcast);
        if_multicast.ipv6.ipv6mr_interface = if_index;
        if (setsockopt(sock, IPPROTO_IPV6, IPV6_MULTICAST_IF, (void *)&if_index, sizeof(if_index)) < 0) {
            fprintf(stderr, "unable to set IPV6_MULTICAST_IF\n");
            exit(-6);
        }
    }

    if (list) free(list);
    return sock;
}
#else
iface_t find_multicast(iface_t list, const char *iface, int family) {
    while (list) {
        const struct sockaddr *addr = NULL;
        if (eq(list->ifa_name, iface) && (list->ifa_flags & IFF_MULTICAST))
            addr = list->ifa_addr;
        if (addr && addr->sa_family == family)
            return list;
        list = list->ifa_next;
    }
    return NULL;
}

int make_multicast(const char *mcast, int family, uint16_t port) {
    int sock = -1;
    iface_t list;
    cpr_memset(&if_multicast, 0, sizeof(if_multicast));
    if (getifaddrs(&list)) {
        perror("getifaddr");
        exit(-1);
    }

    iface_t iface = find_multicast(list, mcast, family);
    if (!iface) {
        fprintf(stderr, "%s: iface not found\b", mcast);
        exit(-2);
    }

    sock = socket(family, SOCK_DGRAM, 0);
    if (sock < 0) {
        fprintf(stderr, "unable to make socket\n");
        exit(-3);
    }

    int reuse = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        fprintf(stderr, "unable to reuse socket\n");
        exit(-4);
    }

    if (family == AF_INET) {
        struct sockaddr_in mcaddr;
        cpr_memset(&mcaddr, 0, sizeof(mcaddr));
        mcaddr.sin_family = AF_INET;
        mcaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        mcaddr.sin_port = htons(port);
        if (bind(sock, (struct sockaddr *)&mcaddr, sizeof(mcaddr))) {
            fprintf(stderr, "unable to BIND socket\n");
            exit(-5);
        }
        const struct sockaddr_in *target = (struct sockaddr_in *)iface->ifa_addr;
        struct in_addr if_addr = target->sin_addr;
        if_multicast.ipv4.imr_interface = if_addr;
        if (setsockopt(sock, IPPROTO_IP, IP_MULTICAST_IF, (void *)&if_addr, sizeof(if_addr))) {
            fprintf(stderr, "unable to set IP_MULTICAST_IF\n");
            exit(-6);
        }
    } else if (family == AF_INET6) {
        struct sockaddr_in6 mcaddr6;
        cpr_memset(&mcaddr6, 0, sizeof(mcaddr6));
        mcaddr6.sin6_family = AF_INET6;
        mcaddr6.sin6_addr = in6addr_any; // wildcard bind
        mcaddr6.sin6_port = htons(port);
        if (bind(sock, (struct sockaddr *)&mcaddr6, sizeof(mcaddr6)) < 0) {
            fprintf(stderr, "unable to BIND IPv6 socket\n");
            exit(-5);
        }

        // Scope outbound multicast to the selected interface
        unsigned if_index = if_nametoindex(mcast);
        if_multicast.ipv6.ipv6mr_interface = if_index;
        if (setsockopt(sock, IPPROTO_IPV6, IPV6_MULTICAST_IF, &if_index, sizeof(if_index)) < 0) {
            fprintf(stderr, "unable to set IPV6_MULTICAST_IF\n");
            exit(-6);
        }
    }

    if (list)
        freeifaddrs(list);
    return sock;
}
#endif

int join_multicast(int so, const struct sockaddr *member) {
    int res = 0;
    multicast_t multicast = if_multicast;

    if (so < 0) return EBADF;
    switch (member->sa_family) {
    case AF_INET:
        multicast.ipv4.imr_multiaddr = to_in4(member)->sin_addr;
        if (setsockopt(so, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void *)&multicast, sizeof(multicast.ipv4)) == -1)
            res = errno;
        break;
    case AF_INET6:
        multicast.ipv6.ipv6mr_multiaddr = to_in6(member)->sin6_addr;
        if (setsockopt(so, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, (void *)&multicast, sizeof(multicast.ipv6)) == -1)
            res = errno;
        break;
    default:
        res = EAI_FAMILY;
    }
    return res;
}

int drop_multicast(int so, const struct sockaddr *member) {
    int res = 0;
    multicast_t multicast = if_multicast;

    if (so < 0) return EBADF;
    switch (member->sa_family) {
    case AF_INET:
        multicast.ipv4.imr_multiaddr = to_in4(member)->sin_addr;
        if (setsockopt(so, IPPROTO_IP, IP_DROP_MEMBERSHIP, (void *)&multicast, sizeof(multicast.ipv4)) == -1)
            res = errno;
        break;
    case AF_INET6:
        multicast.ipv6.ipv6mr_multiaddr = to_in6(member)->sin6_addr;
        if (setsockopt(so, IPPROTO_IPV6, IPV6_DROP_MEMBERSHIP, (void *)&multicast, sizeof(multicast.ipv6)) == -1)
            res = errno;
        break;
    default:
        res = EAI_FAMILY;
    }
    return res;
}
