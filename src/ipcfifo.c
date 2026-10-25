// SPDX-License-Identifier: BSD-3-Clause
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#include <cpr/system.h>
#include <cpr/strchar.h>
#include <cpr/memory.h>
#include <defines.h>
#include <getopt.h>

int main(int argc, char *argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "")) != -1) {
        switch (opt) {
        default:
        usage:
            cpr_crit(-1, "usage: ipcfifo fifo ipc-message [...]");
        }
    }
    if (optind > (argc - 2)) goto usage;

    char *name = argv[optind++];
    char path[256];
    if (!cpr_fifopath(path, sizeof(path), name, false)) cpr_crit(-1, "invalid service: %s", name);
    if (access(path, W_OK)) cpr_crit(-1, "fatal: not active: %s", name);

    char line[256];
    if (!is_text(argv[optind])) goto badarg;
    size_t len = snprintf(line, sizeof(line), "%s", argv[optind++]);
    while (optind < argc && len < sizeof(line)) {
        size_t max = sizeof(line) - len;
        if (!is_text(argv[optind])) goto badarg;
        len += snprintf(line + len, max, "\t%s", argv[optind++]);
    }
    if (len >= sizeof(line)) cpr_crit(-1, "fatal: invalid command");
    line[len++] = '\n';

    int fifo = open(path, O_WRONLY);
    if (fifo < 0) cpr_crit(-1, "cannot open: %s", name);
    if (write(fifo, line, len) != len) cpr_crit(-1, "cannot send:  %s", name);
#ifndef _WIN32
    fsync(fifo);
#endif
    close(fifo);
    return 0;

badarg:
    cpr_crit(-1, "fatal: non text argument");
}
