// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#include <local/xdg.h>
#include <cpr/system.h>
#include <getopt.h>

int main(int argc, char *argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "")) != -1) {
        switch (opt) {
        default:
        usage:
            cpr_crit(-1, "usage: xdgtype");
        }
    }
    if (optind != argc) goto usage;
    printf("%s\n", xdg_desktop());
    return 0;
}
