// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#include <local/vpn.h>
#include <local/net.h>
#include <cpr/system.h>
#include <cpr/strchar.h>
#include <cpr/address.h>
#include <defines.h>
#include <getopt.h>

int main(int argc, char *argv[]) {
    struct sockaddr_storage resolved;
    const char *proxy = cpr_getenv("PROXY", 255);
    const char *host = NULL;
    const char *service = NULL;
    bool ifind = false;
    int family = TUNE_FAMILY_INET;
    int type = SOCK_STREAM;
    int opt;
    while ((opt = getopt(argc, argv, "i46d")) != -1) {
        switch (opt) {
        case 'i':
            ifind = true;
            break;
        case '4':
            family = AF_INET;
            break;
        case '6':
            family = AF_INET6;
            break;
        case 'd':
            type = SOCK_DGRAM;
            break;
        default:
        usage:
            cpr_crit(-1, "usage: hostaddr [options] [host [service]]");
        }
    }

    cpr_sockinit();
    if (optind < argc) host = argv[optind++];
    if (optind < argc) service = argv[optind++];
    if (optind < argc || !host) goto usage;
    if (!cpr_gethost(host, service, family, type, &resolved)) cpr_crit(1, "fatal: unknown host or service: %s", host);

    uint16_t port = cpr_getport(&resolved);
    const char *addr = cpr_getaddr(&resolved);
    if (!addr) cpr_crit(-3, "fatal: not decodable: %s", host);
    if (ifind) {
        if (vpn_detect((const struct sockaddr *)&resolved)) {
            printf("secure\n");
            return 0;
        }

        char buf[IFACE_SIZE];
        if (net_getiface(&resolved, buf, sizeof(buf)))
            printf("%s\n", buf);
        else if (!is_empty(proxy))
            printf("proxy\n");
        else
            printf("default\n");
        return 0;
    }

    if (port)
        printf("%s %d\n", addr, port);
    else
        printf("%s\n", addr);
    return 0;
}
