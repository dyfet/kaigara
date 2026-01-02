// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#include <cpr/system.h>
#include <siot/console.h>
#include <getopt.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    bool clear = true;
    int opt;
    while ((opt = getopt(argc, argv, "r")) != -1) { // FlawFinder: ignore
        switch (opt) {
        case 'r':
            clear = false;
            break;
        default:
        usage:
            cpr_crit(-1, "Usage: cls [-r]");
        }
    }
    if (optind != argc)
        goto usage;

    if (clear) sio_cls();
    sio_reset();
    sio_wrap(true);
    return 0;
}
