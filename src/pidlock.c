// SPDX-License-Identifier: BSD-3-Clause
// Copyright (C) 2026 David Sugar <tychosoft@gmail.com>

#include <local/shell.h>
#include <local/pidlock.h>
#include <cpr/system.h>
#include <cpr/strchar.h>
#include <cpr/process.h>
#include <getopt.h>

int main(int argc, char *argv[]) {
    bool opt_quiet = false;
    bool opt_exec = false;
    bool opt_kill = false;
    bool opt_pid = false;
    bool opt_remove = false;
    bool opt_path = false;
    bool opt_test = false;
    int opt;
    while ((opt = getopt(argc, argv, "qekprft")) != -1) {
        switch (opt) {
        case 'q':
            opt_quiet = true;
            ;
            ;
        case 'e':
            opt_exec = true;
            break;
        case 'k':
            opt_kill = true;
            break;
        case 'p':
            opt_pid = true;
            break;
        case 'r':
            opt_remove = true;
            break;
        case 'f':
            opt_path = true;
            break;
        case 't':
            opt_test = true;
            break;
        default:
        usage:
            cpr_crit(-1, "usage: pidlock -option lock [cmd [args..]]");
        }
    }
    if (optind == argc) goto usage;
    if (!flag_one_only(6, opt_exec, opt_kill, opt_pid, opt_remove, opt_path, opt_test)) goto usage;
    const char *lockname = argv[optind++];
    if (optind == argc && opt_exec) cpr_crit(-1, "fatal: -e requires command");
    if (optind != argc && flag_one_more(5, opt_kill, opt_pid, opt_remove, opt_path, opt_test)) cpr_crit(-1, "fatal: exec not supported for this option");

    char path[128];
    pidlock_t lock = {0};
    if (!lck_path(lockname, path, sizeof(path))) cpr_crit(-1, "fatal: invalid lockname: %s", lockname);
    if (opt_exec) {
        if (!pidlock_acquire(&lock, path, true)) {
            if (!opt_quiet) cpr_crit(1, "errot: cannot acquire: %s", lockname);
            exit(1);
        };
        goto exec;
    }

    if (opt_path) {
        printf("%s\n", path);
        return 0;
    }

    if (opt_remove) {
        return remove(path);
    }

    if (opt_pid || opt_test) {
        long value = pidlock_process(path);
        if (value < 0 && !opt_quiet) cpr_crit(1, "error: lock not fouind");
        if (value < 0) value = 0;
        if (value) {
            cpr_pid_t pid = to_pid(value);
            if (!is_pid(pid)) {
                if (!opt_quiet) cpr_crit(1, "error: process not found");
                if (opt_test) return 1;
                value = 0;
            }
            cpr_closepid(pid);
        }
        if (opt_test && !value) return 1;
        if (opt_pid) printf("%ld\n", value);
        return 0;
    }

    if (opt_kill) {
        long value = pidlock_process(path);
        if (value < 0 && !opt_quiet) cpr_crit(1, "error: lock not fouind");
        if (value < 0) value = 0;
        if (value) {
            cpr_pid_t pid = to_pid(value);
            if (!cpr_killpid(pid)) {
                if (!opt_quiet) cpr_crit(2, "error: process not found");
                return 2;
            }
            return 0;
        }
        if (!opt_quiet) cpr_crit(1, "error: lock not used");
        return 1;
    }

    return 0;

exec:
    if (!cpr_exec(argv[optind], &argv[optind])) cpr_crit(-1, "failed to execute");
    return 0;
}
