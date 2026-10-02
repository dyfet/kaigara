// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 202y David Sugar <tychosoft@gmail.com>

#include <cpr/memory.h>
#include <cpr/strchar.h>
#include <errno.h>
#include "net.h"
#include "vpn.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#pragma comment(lib, "iphlpapi.lib")
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#endif

static bool is_tailscale_ipv4(const struct sockaddr *sa) {
    if (!sa || sa->sa_family != AF_INET) return false;
    const struct sockaddr_in *sin = (const struct sockaddr_in *)sa;
    uint32_t ip = ntohl(sin->sin_addr.s_addr);
    return (ip & 0xFFC00000u) == 0x64400000u; // 100.64.0.0/10
}

#if defined(__linux__)
#include <unistd.h>
#include <linux/rtnetlink.h>
#include <net/if.h>
#include <sys/socket.h>

#define HAS_DETECT 1

static int get_route_ifindex(const struct sockaddr *sa) {
    struct {
        struct nlmsghdr nh;
        struct rtmsg rt;
        char buf[256];
    } req;

    int fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (fd < 0) return -1;
    cpr_memset(&req, 0, sizeof(req));
    req.nh.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
    req.nh.nlmsg_type = RTM_GETROUTE;
    req.nh.nlmsg_flags = NLM_F_REQUEST;
    req.rt.rtm_family = sa->sa_family;

    struct rtattr *rta = (struct rtattr *)(((char *)&req) + NLMSG_ALIGN(req.nh.nlmsg_len));
    int alen = 0;
    if (sa->sa_family == AF_INET) {
        const struct sockaddr_in *sin = (const struct sockaddr_in *)sa;
        rta->rta_type = RTA_DST;
        rta->rta_len = RTA_LENGTH(4);
        cpr_memcpy(RTA_DATA(rta), 4, &sin->sin_addr, 4);
        alen = RTA_LENGTH(4);
        req.rt.rtm_dst_len = 32;
    } else if (sa->sa_family == AF_INET6) {
        const struct sockaddr_in6 *sin6 = (const struct sockaddr_in6 *)sa;
        rta->rta_type = RTA_DST;
        rta->rta_len = RTA_LENGTH(16);
        cpr_memcpy(RTA_DATA(rta), 16, &sin6->sin6_addr, 16);
        alen = RTA_LENGTH(16);
        req.rt.rtm_dst_len = 128;
    }

    struct sockaddr_nl nladdr;
    req.nh.nlmsg_len = NLMSG_ALIGN(req.nh.nlmsg_len) + alen;
    cpr_memset(&nladdr, 0, sizeof(nladdr));
    nladdr.nl_family = AF_NETLINK;
    if (sendto(fd, &req, req.nh.nlmsg_len, 0,
        (struct sockaddr *)&nladdr, sizeof(nladdr)) < 0) {
        close(fd);
        return -1;
    }

    char buf[4096];
    ssize_t len = recv(fd, buf, sizeof(buf), 0);
    close(fd);
    if (len < 0) return -1;

    struct nlmsghdr *nh;
    for (nh = (struct nlmsghdr *)buf; NLMSG_OK(nh, len); nh = NLMSG_NEXT(nh, len)) {
        if (nh->nlmsg_type == NLMSG_ERROR) return -1;
        if (nh->nlmsg_type != RTM_NEWROUTE) continue;

        struct rtmsg *rt = NLMSG_DATA(nh);
        struct rtattr *attr = RTM_RTA(rt);
        int attrlen = RTM_PAYLOAD(nh);
        for (; RTA_OK(attr, attrlen); attr = RTA_NEXT(attr, attrlen)) {
            if (attr->rta_type == RTA_OIF) {
                return *(int *)RTA_DATA(attr);
            }
        }
    }
    return -1;
}

static bool is_tunnel_route(const struct sockaddr *sa) {
    int ifindex = get_route_ifindex(sa);
    if (ifindex <= 0) return false;

    char ifname[IF_NAMESIZE] = {0};
    if (!if_indextoname(ifindex, ifname)) return false;
    if (strncmp(ifname, "tailscale", 9) == 0) return true;
    if (strncmp(ifname, "wg", 2) == 0) return true;
    return false;
}

#endif

#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || \
defined(__NetBSD__) || defined(__DragonFly__)
#include <unistd.h>
#include <net/route.h>
#include <net/if.h>

#define HAS_DETECT 1

static int get_route_ifindex(const struct sockaddr *sa) {
    int fd = socket(PF_ROUTE, SOCK_RAW, 0);
    if (fd < 0) return -1;

    char buf[512];
    cpr_memset(buf, 0, sizeof(buf));

    struct rt_msghdr *rtm = (struct rt_msghdr *)buf;
    rtm->rtm_msglen = sizeof(*rtm) + sa->sa_len;
    rtm->rtm_version = RTM_VERSION;
    rtm->rtm_type = RTM_GET;
    rtm->rtm_flags = RTF_UP | RTF_HOST;
    rtm->rtm_addrs = RTA_DST;
    cpr_memcpy(buf + sizeof(*rtm), sizeof(buf) - sizeof(*rtm), sa, sa->sa_len);
    if (write(fd, rtm, rtm->rtm_msglen) < 0) {
        close(fd);
        return -1;
    }

    int len = read(fd, buf, sizeof(buf));
    close(fd);
    if (len < (int)sizeof(*rtm)) return -1;
    rtm = (struct rt_msghdr *)buf;
    if (rtm->rtm_type != RTM_GET) return -1;
    return rtm->rtm_index;
}

static bool is_tunnel_route(const struct sockaddr *sa) {
    int ifindex = get_route_ifindex(sa);
    if (ifindex <= 0) return false;

    char ifname[IF_NAMESIZE] = {0};
    if (!if_indextoname(ifindex, ifname)) return false;
    if (strncmp(ifname, "tailscale", 9) == 0) return true;
    if (strncmp(ifname, "wg", 2) == 0) return true;
    return false;
}

#endif

#if defined(_WIN32)
#include <stdlib.h>

#define HAS_DETECT 1

static bool is_tunnel_route(const struct sockaddr *sa) {
    DWORD ifIndex = 0;
    if (GetBestInterfaceEx((SOCKADDR *)sa, &ifIndex) != NO_ERROR) return false;

    ULONG size = 0;
    if (GetAdaptersAddresses(AF_UNSPEC, 0, NULL, NULL, &size) != ERROR_BUFFER_OVERFLOW) return false;
    IP_ADAPTER_ADDRESSES *addrs = (IP_ADAPTER_ADDRESSES *)malloc(size);
    if (!addrs) return false;

    DWORD r = GetAdaptersAddresses(AF_UNSPEC, 0, NULL, addrs, &size);
    if (r != NO_ERROR) {
        free(addrs);
        return false;
    }

    bool result = false;
    for (IP_ADAPTER_ADDRESSES *a = addrs; a; a = a->Next) {
        if (a->IfIndex != ifIndex && a->Ipv6IfIndex != ifIndex) continue;
        if (a->IfType == IF_TYPE_TUNNEL) {
            result = true;
            break;
        }

        if (a->FriendlyName) {
            if (wcsstr(a->FriendlyName, L"Tailscale")) {
                result = true;
                break;
            }
            if (wcsstr(a->FriendlyName, L"WireGuard")) {
                result = true;
                break;
            }
            if (wcsstr(a->FriendlyName, L"wg")) {
                result = true;
                break;
            }
        }
    }

    free(addrs);
    return result;
}
#endif

bool vpn_detect(const struct sockaddr *sa) {
    if (!sa) return false;
    if (is_tailscale_ipv4(sa)) return true;
#ifdef HAS_DETECT
    return is_tunnel_route(sa);
#else
    return false;
#endif
}

int vpn_connect(const struct sockaddr *addr) {
#ifdef _WJB32
    const char *ident = cpr_getenv("USERNAME", 256);
#else
    const char *ident = cpr_getenv("USER", 256);
#endif

    const char *proxy = cpr_getenv("PROXY", 256);
    uint16_t port = 1080;

    // if no proxy, may as well just connect directly
    if (is_empty(proxy)) return net_connect(addr);

    // if secure vpn, we may also connect directly
    if (vpn_detect(addr)) return net_connect(addr);

    // if its on a local subnet, we connect direct
    if (net_getiface((const sockaddr_t *)addr, NULL, 0)) return net_connect(addr);

    // use socks outbound proxy
    return net_proxy(proxy, port, ident, addr);
}
