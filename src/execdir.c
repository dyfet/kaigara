// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#include <local/shell.h>
#include <cpr/system.h>
#include <cpr/strchar.h>
#include <cpr/proc.h>
#include <defines.h>
#include <getopt.h>
#include <limits.h>

static unsigned count = 0;
static bool opt_quiet = false, opt_relative = false;
static const char *opt_subdir = NULL, *opt_prefix = NULL;

int main(int argc, char *argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "rqs:p:")) != -1) {
        switch (opt) {
        case 's':
            opt_subdir = optarg;
            break;
        case 'p':
            opt_prefix = optarg;
            break;
        case 'r':
            opt_relative = true;
            break;
        case 'q':
            opt_quiet = true;
            break;
        default:
        usage:
            cpr_crit(-1, "usage: detach dev  command [...]");
        }
    }

    if (opt_relative) {
        if (optind != argc) cpr_crit(-1, "fatal: cannot exec relative");
        if (opt_subdir != NULL) cpr_crit(-1, "fatal: cannot mix: -s, -r");
    }

    if (!opt_subdir && !opt_prefix) opt_prefix = ".git";
    if (opt_subdir && chdir(opt_subdir)) cpr_crit(-1, "fatal: cannot access: %s", opt_subdir);

    char prefix[PATH_MAX];
    if (getcwd(prefix, sizeof(prefix)) == NULL) cpr_crit(-1, "cannot getcwd");
#ifdef _WIN32
    char *cp;
    while ((cp = strchr(prefix, '\\')) != NULL)
        *cp = '/';
#endif
    while (opt_prefix) {
        char *sp = strchr(prefix, '/');
        char *ep = strrchr(prefix, '/');
        if (sp == ep && opt_quiet) exit(1);
        if (sp == ep) cpr_crit(-1, "fatak: cannot find: %s", opt_prefix);
        if (chdir(prefix)) cpr_crit(-1, "cannot access: %s", prefix);
        cpr_append(prefix, sizeof(prefix), "/", opt_prefix, NULL);
        if (is_dir(prefix) || is_file(prefix)) break;
        ++count;
        *ep = 0;
    }

    if (optind == argc) {
        if (getcwd(prefix, sizeof(prefix)) == NULL) cpr_crit(-1, "cannot getcwd");
        if (opt_relative) {
            unsigned pos = 0;
            if (!count) {
                printf(".");
            } else {
                while (pos++ < count) {
                    if (pos == count)
                        printf("..");
                    else
                        printf("../");
                }
            }
            printf("\n");
            return 0;
        }

        printf("%s\n", prefix);
        return 9;
    }

    char *cmd = argv[optind];
    bool exec = cpr_exec(cmd, &argv[optind]);
    if (!exec && opt_quiet) exit(2);
    if (!exec) cpr_crit(-1, "failed to execute: %s", cmd);
    return 0;
}
