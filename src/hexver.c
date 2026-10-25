// SPDX-License-Identifier: BSD-3-Clause
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#include <local/ver.h>
#include <cpr/system.h>
#include <cpr/strchar.h>
#include <defines.h>
#include <getopt.h>

static ver_t ver;

int main(int argc, char *argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "")) != -1) {
        switch (opt) {
        default:
        usage:
            cpr_crit(-1, "usage: hexver version string");
        }
    }
    if (optind != (argc - 1)) goto usage;
    if (!ver_get(&ver, argv[optind]))
        cpr_crit(1, "fatal: invalid version: %s", argv[optind]);

    char hex[33];
    ver_hex(hex, sizeof(hex), &ver);
    printf("%s\n", hex);
    return 0;
}
