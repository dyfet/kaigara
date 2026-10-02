// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#include <local/scan.h>
#include <cpr/system.h>
#include <minicrypt/random.h>
#include <getopt.h>
#include <stdlib.h>

static bool opt_lines = false;
static unsigned opt_count = 1;
static unsigned opt_rounds = 1;
static uint64_t opt_min, opt_max;

int main(int argc, char *argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "r:c:l")) != -1) {
        switch (opt) {
        case 'l':
            opt_lines = true;
            break;
        case 'c':
            opt_count = scan_value(optarg, 1, 1999);
            break;
        case 'r':
            opt_rounds = scan_value(optarg, 1, 1000);
            break;
        default:
        usage:
            cpr_crit(-1, "usage: rand [options] min max");
        }
    }
    if (optind < argc)
        opt_min = scan_value(argv[optind++], 0, 1000);
    else
        goto usage;

    if (optind < argc)
        opt_max = scan_value(argv[optind++], 1, 1000);
    else
        goto usage;

    if (optind < argc) goto usage;
    if (opt_max <= opt_min) {
        fprintf(stderr, "%s: invalid min or max: %d<%d\n", argv[0], (int)opt_min, (int)opt_max);
        goto usage;
    }

    if (!opt_count) {
        fprintf(stderr, "%s: count invalid: %d\n", argv[0], opt_count);
        goto usage;
    }

    if (!opt_rounds) {
        fprintf(stderr, "%s: rounds invalid: %d\n", argv[0], opt_rounds);
        goto usage;
    }

    mc_random_ctx ctx;
    char *space = "";
    mc_random_init(&ctx);
    while (opt_count--) {
        unsigned sums = opt_rounds;
        uint64_t out = 0;
        while (sums--)
            out += mc_uniform_random(&ctx, opt_min, opt_max);
        if (opt_lines)
            printf("%lu\n", (long unsigned)out);
        else {
            printf("%s%lu", space, (long unsigned)out);
            space = " ";
        }
    }
    if (!opt_lines) printf("\n");
    mc_random_free(&ctx);
    return 0;
}
