// SPDX-License-Identifier: BSD-3-Clause
// Copyright (C) 2026 David Sugar <tychosoft@gmail.com>

#include <local/shell.h>
#include <cpr/system.h>
#include <cpr/strchar.h>
#include <cpr/process.h>
#include <getopt.h>

int main(int argc, char *argv[]) {
    bool opt_quiet = false;
    int opt;
    while ((opt = getopt(argc, argv, "q")) != -1) {
        switch (opt) {
        case 'q':
            opt_quiet = true;
            break;
        default:
        usage:
            cpr_crit(-1, "usage: trypath [option] name(s) [- [args..]]");
        }
    }
    if (optind == argc) goto usage;

    const char *env = getenv("PATH");
    if (!env || !*env) cpr_crit(-1, "fatal: no path");

    char delim[2] = {LIST_SEP, 0};
    char *execpath = NULL;
    while (optind < argc) {
        if (eq(argv[optind], "-") || eq(argv[optind], "--")) break;
        if (!execpath) {
            char *path = strdup(env);
            char *mark = NULL;
            bool first = true;
            const char *arg = argv[optind];
            while (!execpath) {
                const char *tok = cpr_strtok(first ? path : NULL, delim, &mark);
                first = false;
                if (!tok) break;
                if (!cpr_asprintf(&execpath, "%s%c%s", tok, PATH_SEP, arg)) break;
#ifdef _WIN32
                char extpath[strlen(execpath) + 5];
                snprintf(extpath, sizeof(extpath), "%s.exe", execpath);
                if (is_file(extpath)) break;
                snprintf(extpath, sizeof(extpath), "%s.bat", execpath);
                if (is_file(extpath)) break;
                snprintf(extpath, sizeof(extpath), "%s.ps1", execpath);
                if (is_file(extpath)) break;
#else
                if (is_file(execpath)) break;
#endif
                free(execpath);
                execpath = NULL;
            }
            free(path);
        }
        ++optind;
    }

    if (!execpath) {
        if (opt_quiet) return 1;
        cpr_crit(1, "fatal: no path found");
    }

    if (optind == argc) {
        printf("%s\n", execpath);
        return 0;
    }

    argv[optind] = execpath;
    bool exec = cpr_exec(execpath, &argv[optind]);
    if (!exec && opt_quiet) exit(2);
    if (!exec) cpr_crit(-1, "failed to execute: %s", execpath);
    return 0;
}
