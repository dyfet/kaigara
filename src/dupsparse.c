// SPDX-License-Identifier: BSD-3-Clause
// Copyright (C) 2026 David Sugar <tychosoft@gmail.com>

#include <local/shell.h>
#include <local/sparse.h>
#include <cpr/system.h>
#include <getopt.h>

int main(int argc, char *argv[]) {
    bool opt_force = false;
    int opt;
    while ((opt = getopt(argc, argv, "f")) != -1) {
        switch (opt) {
        case 'f':
            opt_force = true;
            break;
        default:
        usage:
            cpr_crit(-1, "usage: dupsparse [option] old new");
        }
    }

    if (optind == argc) goto usage;

    const char *old = argv[optind++];
    if (optind == argc) goto usage;

    const char *new = argv[optind++];
    if (optind != argc) goto usage;
    if (opt_force && !cpr_rmfile(new)) cpr_crit(-1, "fatal: force: %s", new);
    if (!dupsparse(old, new, 412)) cpr_crit(-1, "fatal: cannot dup");
    return 0;
}
