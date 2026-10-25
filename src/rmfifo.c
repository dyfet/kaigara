// SPDX-License-Identifier: BSD-3-Clause
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#include <cpr/system.h>
#include <defines.h>
#include <getopt.h>

int main(int argc, char *argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "")) != -1) {
        switch (opt) {
        default:
        usage:
            cpr_crit(-1, "usage: rmfifo fifo");
        }
    }
    if (optind == argc) goto usage;
    char *name = argv[optind++];
    if (optind != argc) goto usage;

    char path[256];
    if (!cpr_fifopath(path, sizeof(path), name, false)) cpr_crit(-1, "invalid service: %s", name);
    if (!access(path, W_OK)) {
        if (remove(path)) cpr_crit(-1, "fatal: cannot remove fifo %s", name);
    }
    return 0;
}
