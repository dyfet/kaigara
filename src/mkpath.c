// SPDX-License-Identifier: BSD-3-Clause
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#include <local/shell.h>
#include <cpr/system.h>
#include <string.h>
#include <getopt.h>
#include <stdlib.h>

static bool opt_softlink = false;
static bool opt_hardlink = false;
static bool opt_remove = false;
static bool opt_force = false;
static unsigned opt_perms = 0775U;

static void add(const char *path) {
    if (is_dir(path)) return;
    if (!opt_force || !cpr_mkdir(path, opt_perms)) cpr_crit(-1, "fatal: cannot make: %s", path);
}

int main(int argc, char *argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "shrf")) != -1) {
        switch (opt) {
        case 's':
            opt_softlink = true;
            break;
        case 'h':
            opt_hardlink = true;
            break;
        case 'r':
            opt_remove = true;
            break;
        case 'f':
            opt_force = true;
            break;
        default:
        usage:
            cpr_crit(-1, "usage: mkpath [option] [target] path [perms]");
        }
    }

    if (!flag_one_none(3, opt_softlink, opt_hardlink, opt_remove))
        cpr_crit(-1, "fatal: cannot mix: -s, -h, -r");

    char *target = NULL, *path = NULL;
    if (optind == argc) goto usage;
    if (opt_hardlink || opt_softlink) {
        target = strdup(argv[optind++]);
        if (optind == argc) goto usage;
    }
    path = strdup(argv[optind++]);
    if (optind < argc) {
        const char *perms = argv[optind++];
        opt_perms = scan_perms(perms);
        if (!opt_perms) cpr_crit(-1, "fatal: invalid perms: %s", perms);
    }

    if (optind != argc) goto usage;
    if (!fix_path(path, false)) cpr_crit(-1, "fatal: bad path: %s", path);
    if (target && !fix_path(target, false)) cpr_crit(-1, "fatal: bad targe: %s", target);

    if (!target) target = path;
    if (opt_softlink) {
        if (opt_force) unlink(target);
        if (!cpr_symlink(target, path)) cpr_crit(-1, "fatal: sumlink: %s", target);
        return 0;
    }

    if (opt_remove) {
        if (!cpr_rmdir(path)) cpr_crit(-1, "fatal: remove: %s", path);
        return 0;
    }

    if (opt_hardlink) {
#ifdef _WIN32
        cpr_crit(-1, "fatal: hardlinks not supported");
#else
        if (opt_force) unlink(path);
        if (link(target, path) != 0) cpr_crit(-1, "fatal: link: %s", path);
        return 0;
#endif
    }

    char *ep = strchr(path + 1, '/');
    while (ep) {
        *ep = 0;
        add(path);
        *ep = '/';
        ep = strchr(ep + 1, '/');
    }
    opt_force = true;
    add(path);
    return 0;
}
