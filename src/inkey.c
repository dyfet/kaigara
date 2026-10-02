// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#include <local/con.h>
#include <local/scan.h>
#include <cpr/system.h>
#include <cpr/strchar.h>
#include <defines.h>
#include <getopt.h>

static int opt_timeout = 1000;

int main(int argc, char *argv[]) {
    int opt, line, col;
    while ((opt = getopt(argc, argv, "t:")) != -1) {
        switch (opt) {
        case 't':
            opt_timeout = scan_value(optarg, 1, 60);
            if (!opt_timeout) cpr_crit(-1, "fatal: bad timeout: %s", optarg);
            opt_timeout *= 1000;
            break;
        default:
        usage:
            cpr_crit(-1, "usage: inkey [option]");
        }
    }

    if (optind < argc) goto usage;
    if (!con_startup(KEY_INPUT_DEFAULT)) cpr_crit(-1, "fatal: no console");
    atexit(con_shutdown);
    int key = con_inkey(opt_timeout);
    if (key == 0x1b) con_ignore();
    printf("%d\n", key);
    exit(0);
}
