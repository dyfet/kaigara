// SPDX-License-Identifier: BSD-3-Clause
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#include <local/con.h>
#include <local/scan.h>
#include <cpr/system.h>
#include <cpr/strchar.h>
#include <defines.h>
#include <getopt.h>

static const char *opt_prompt = NULL;
static int opt_timeout = 1000;

int main(int argc, char *argv[]) {
    int opt, line, col;
    while ((opt = getopt(argc, argv, "p:t:")) != -1) {
        switch (opt) {
        case 'p':
            opt_prompt = optarg;
            break;
        case 't':
            opt_timeout = scan_value(optarg, 1, 60);
            if (!opt_timeout) cpr_crit(-1, "fatal: bad timeout: %s", optarg);
            opt_timeout *= 1000;
            break;
        default:
        usage:
            cpr_crit(-1, "usage: choose [option] choices");
        }
    }

    if (optind == argc) goto usage;
    const char *choices = argv[optind++];

    if (optind != argc) goto usage;
    if (!con_startup(KEY_INPUT_DEFAULT)) cpr_crit(-1, "fatal: no console");
    atexit(con_shutdown);
    exit(con_choose(opt_prompt, choices, opt_timeout, true));
}
