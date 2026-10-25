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
            cpr_crit(-1, "usage: mksparse [option] path size [perms]");
        }
    }

    if (optind == argc) goto usage;

    const char *path = argv[optind++];
    if (optind == argc) goto usage;

    uint64_t size = scan_size(argv[optind++]);
    if (size == bad_size) cpr_crit(-1, "fatal: size: %s", argv[--optind]);

    unsigned perms = 0644U;
    if (optind != argc) perms = scan_perms(argv[optind++]);
    if (perms == bad_perms) cpr_crit(-1, "fatal: perms: %s", argv[--optind]);
    if (opt_force && !cpr_rmfile(path)) cpr_crit(-1, "fatal: force: %s", path);
    if (!mksparse(path, size, perms)) cpr_crit(-1, "fatal: sparse: %s", path);
    if (optind != argc) goto usage;
    return 0;
}
