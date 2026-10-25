// SPDX-License-Identifier: BSD-3-Clause
// Copyright (C) 2026 David Sugar <tychosoft@gmail.com>

#include <local/vpn.h>
#include <local/scan.h>
#include <cpr/system.h>
#include <cpr/strchar.h>
#include <defines.h>
#include <getopt.h>

#define MAX_USERID 64

static int opt_family = TUNE_FAMILY_INET;
static uint16_t opt_port = 0;
static char *opt_host = NULL;
static char *opt_user = NULL;
static sockaddr_t tcp_host;

int main(int argc, char *argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "46hp:")) != -1) {
        switch (opt) {
        case '4':
            opt_family = AF_INET;
            break;
        case '6':
            opt_family = AF_INET6;
            break;
        case 'p':
            opt_port = (uint16_t)scan_value(optarg, 1, 49151);
            break;
        default:
        usage:
            cpr_crit(-1, "usage: finger [options] user@host|host");
        }
    }

    cpr_sockinit();
    if (optind != (argc - 1)) goto usage;
    if (!opt_port) opt_port = 78;
    opt_user = argv[optind++];
    opt_host = strchr(opt_user, '@');
    if (!opt_host) {
        opt_host = opt_user;
        opt_user = "";
    } else {
        *(opt_host++) = 0;
    }
    if (!net_service(opt_family, SOCK_STREAM, opt_host, "finger", opt_port, &tcp_host)) cpr_crit(1, "fatal: unknown fibger: %s", opt_host);

    int so = vpn_connect((struct sockaddr *)&tcp_host);
    if (so < 0) cpr_crit(2, "fatal: cannot connect: %s", opt_host);
    if (net_printf(so, MAX_USERID + 3, "%s\r\n", opt_user))
        cpr_crit(3, "fatal: userid error: %s", opt_host);

    char buf[1024];
    ssize_t len = 1;
    do {
        len = recv(so, buf, sizeof(buf), MSG_WAITALL);
        if (len < 0) cpr_crit(4, "fatal: read error: %s", opt_host);
        if (len) fwrite(buf, len, 1, stdout);
    } while (len > 0);
    fflush(stdout);
    cpr_sockclose(so);
    return 0;
}
