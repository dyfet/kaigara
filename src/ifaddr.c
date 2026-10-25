// SPDX-License-Identifier: BSD-3-Clause
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#include <cpr/system.h>
#include <cpr/strchar.h>
#include <cpr/address.h>
#include <defines.h>
#include <getopt.h>

int main(int argc, char *argv[]) {
    char *name = NULL;
    int family = TUNE_FAMILY_INET;
    int opt;
    while ((opt = getopt(argc, argv, "46")) != -1) {
        switch (opt) {
        case '4':
            family = AF_INET;
            break;
        case '6':
            family = AF_INET6;
            break;
        default:
        usage:
            cpr_crit(-1, "usage: ifwaddr [-6] ifname");
        }
    }

    cpr_sockinit();
    if (optind != (argc - 1)) goto usage;
    name = argv[optind];

    sockaddr_t ifa;
    if (!cpr_getbind(name, family, &ifa, 0))
        cpr_crit(-2, "fatal: interface not found: %s", name);
    const char *addr = cpr_getaddr(&ifa);
    if (!addr) cpr_crit(-3, "fatal: not decodab: %sle", name);
    printf("%s\n", addr);
    return 0;
}
