// SPDX-License-Identifier: BSD-3-Clause
// Copyright (C) !!YEAR!! !!AUTHOR!! <!!EMAIL!!>

/* skel lib  */

#include "!!HEADER!!"

/* skel src */

#include <cpr/system.h>
#include <getopt.h>

int main(int argc, char *argv[]) {
    int opt;
   while ((opt = getopt(argc, argv, "")) != -1) {
        switch (opt) {
        default:
        usage:
            cpr_crit(-1, "usage: ???");
        }
    }
    if (optind != argc) goto usage;
    return 0;
}


