// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#include <local/con.h>
#include <cpr/system.h>
#include <getopt.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    bool opt_clear = true;
    int opt;
    while ((opt = getopt(argc, argv, "r")) != -1) {
        switch (opt) {
        case 'r':
            opt_clear = false;
            break;
        default:
        usage:
            cpr_crit(-1, "usage: cls [-r]");
        }
    }
    if (optind != argc) goto usage;
    if (opt_clear) con_cls();
    con_reset();
    con_wrap(true);
    return 0;
}
