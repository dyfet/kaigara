// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#include <local/con.h>
#include <cpr/system.h>
#include <getopt.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "")) != -1) {
        switch (opt) {
        default:
        usage:
            cpr_crit(-1, "usage: istty");
        }
    }
    if (optind != argc) goto usage;
    if (con_istty()) return 0;
    return 1;
}
