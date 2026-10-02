// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#include <local/con.h>
#include <local/scan.h>
#include <cpr/system.h>
#include <cpr/strchar.h>
#include <defines.h>
#include <getopt.h>

int main(int argc, char *argv[]) {
    int opt, line, col;
    while ((opt = getopt(argc, argv, "")) != -1) {
        switch (opt) {
        default:
        usage:
            cpr_crit(-1, "usage: echoat line col ...");
        }
    }

    if (optind > argc - 3) goto usage;
    if (!con_startup(KEY_INPUT_DISABLED)) cpr_crit(-1, "fatal: no console");
    atexit(con_shutdown);
    line = scan_value(argv[optind++], 1, con_lines());
    col = scan_value(argv[optind++], 1, con_cols());
    if (!con_goto(line, col)) cpr_crit(-1, "fatal: invalid position");

    bool space = false;
    while (optind < argc) {
        if (space)
            printf(" %s", argv[optind++]);
        else
            printf("%s", argv[optind++]);
    }
    fflush(stdout);
    return 0;
}
