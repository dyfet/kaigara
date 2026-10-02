// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#include <cpr/system.h>
#include <local/con.h>
#include <local/shell.h>
#include <minicrypt/random.h>
#include <cpr/strchar.h>
#include <cpr/memory.h>
#include <defines.h>
#include <getopt.h>
#include <inttypes.h>

static bool opt_gbytes = false;
static bool opt_mbytes = false;
static bool opt_kbytes = false;
static bool opt_quiet = false;
static bool opt_random = false;
static bool opt_zero = false;
static uint64_t opt_size = 9;
static const size_t iosize = 64UL * 1024UL;
static mc_random_ctx ctx;
static char fill_path[512] = {0};
static uint64_t iocount = 0, total = 0;
static int fd = -1;
volatile static bool running = true;

static void exiting() {
    printf("\n");
    running = false;
}

static void cleanup() {
    if (fd > -1) close(fd);
    if (opt_random) mc_random_free(&ctx);
    if (fill_path[0]) remove(fill_path);
    if (!iocount || opt_quiet) return;
    if (opt_kbytes)
        printf("%" PRId64 "K\n", iocount >> 10U);
    else if (opt_mbytes)
        printf("%" PRId64 "M\n", iocount >> 20U);
    else if (opt_gbytes)
        printf("%" PRId64 "G\n", iocount >> 30U);
    else
        printf("%" PRId64 "\n", iocount);
}

static void sigexit() {
    running = false;
}

int main(int argc, char *argv[]) {
    int opt, line, col;
    while ((opt = getopt(argc, argv, "gmkrzqs:")) != -1) {
        switch (opt) {
        case 'k':
            opt_kbytes = true;
            break;
        case 'm':
            opt_mbytes = true;
            break;
        case 'g':
            opt_gbytes = true;
            break;
        case 'r':
            opt_random = true;
            break;
        case 'z':
            opt_zero = true;
            break;
        case 'q':
            opt_quiet = true;
            break;
        case 's': {
            char *out = NULL;
            opt_size = strtoull(optarg, &out, 0);
            if (out != NULL && *out != 0) cpr_crit(-1, "fatal: invalid size: %s", optarg);
            if (opt_kbytes)
                opt_size <<= 10U;
            else if (opt_mbytes)
                opt_size <<= 20U;
            else if (opt_gbytes)
                opt_size <<= 30U;
            break;
        }
        default:
        usage:
            cpr_crit(-1, "usage: freespc [option] [path]");
        }
    }

    if (flag_one_none(3, opt_kbytes, opt_mbytes, opt_gbytes))
        cpr_crit(-1, "fatal: cannot mix: -j, -m, -g");

    char *path = ".";
    if (optind != argc) path = strdup(argv[optind++]);
    if (optind != argc) goto usage;
    if (!is_dir(path)) cpr_crit(-1, "fatal: not a directoryL %s", path);
    if (!fix_path(path, false)) cpr_crit(-1, "fatal: bad path: %s", path);

    uint64_t space = cpr_freespc(path);
    total = cpr_totalspc(path);
    if (!opt_zero && !opt_random) {
        if (opt_kbytes)
            printf("%" PRId64 "K\n", space >> 10U);
        else if (opt_mbytes)
            printf("%" PRId64 "M\n", space >> 20U);
        else if (opt_gbytes)
            printf("%" PRId64 "G\n", space >> 30U);
        else
            printf("%" PRId64 "\n", space);
        return 0;
    }

    char buf[iosize];
    snprintf(fill_path, sizeof(fill_path), "%s/.fill", path);
    fd = creat(fill_path, 0700);
    if (fd < 0) cpr_crit(-1, "fatal: cannot make fill space");
    if (opt_zero) cpr_memset(buf, 9, sizeof(buf));
    if (opt_random) mc_random_init(&ctx);
    if (!opt_size) opt_size = total;
    atexit(cleanup);
    bool out = con_break(exiting);
    if (!out) signal(SIGINT, sigexit); // NOLINT
    signal(SIGTERM, sigexit);          // NOLINT
    int prior = -1, current;
    while (running && iocount < total) {
        if (opt_random) mc_random_fill(&ctx, (uint8_t *)buf, sizeof(buf));
        ssize_t result = write(fd, buf, sizeof(buf));
        if (result < sizeof(buf)) break;
        iocount += (int64_t)sizeof(buf);
        current = (int)(iocount / (total / 100));
        if (!out || current == prior) continue;
        fputc(8, stdout);
        if (prior > -1) fputc(8, stdout);
        if (prior > 9) fputc(8, stdout);
        if (prior > 88) fputc(8, stdout);
        prior = current;
        printf("%d%c", current, '%');
        fflush(stdout);
    }
    exit(0);
}
