// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#include <local/shell.h>
#include <cpr/system.h>
#include <cpr/strchar.h>
#include <getopt.h>
#include <minicrypt/sha1.h>
#include <cpr/memory.h>

static bool opt_hidden = false;

static bool is_hidden(const char *path) {
    const char *cp = strrchr(path, '/');
    if (cp)
        ++cp;
    else
        cp = path;

    if (*cp == '.') return true;
    return false;
}

static void display(char *arg) {
    FILE *fp = NULL;
    char *cp = strrchr(arg, '/');
    if (eq(arg, "-"))
        fp = stdin;
    else
        fp = fopen(arg, "rb");

    if (!fp) {
        fprintf(stderr, "error: fopen: %s\n", arg);
        return;
    }

    uint8_t buf[MC_SHA1_DIGEST_SIZE];
    mc_sha1_ctx ctx;
    mc_sha1_init(&ctx);
    while (!feof(fp) && !ferror(fp)) {
        size_t n = fread(buf, 1, sizeof(buf), fp);
        if (!n) break;
        mc_sha1_update(&ctx, buf, n);
    }

    mc_sha1_final(&ctx, buf);
    if (ferror(fp)) {
        fprintf(stderr, "error: read: %s\n", arg);
        fclose(fp);
        return;
    }

    char *hex = cpr_hexdup(buf, sizeof(buf));
    if (cp) arg = ++cp;
    if (fp == stdin)
        printf("%s\n", hex);
    else
        printf("%s %s\n", hex, arg);
    if (fp) fclose(fp);
    cpr_strfree(&hex);
}

static void scan(const char *path) {
    char buf[512];
    DIR *dir = opendir(path);
    struct dirent *entry;
    if (!dir) {
        perror("dir");
        exit(1);
    }
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.' && !opt_hidden) continue;
        snprintf(buf, sizeof(buf), "%s/%s", path, entry->d_name);
        if (is_dir(buf) || access(buf, R_OK)) continue;
        display(buf);
    }
    closedir(dir);
}

int main(int argc, char *argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "h")) != -1) {
        switch (opt) {
        case 'h':
            opt_hidden = true;
            break;
        default:
        usage:
            cpr_crit(-1, "usage: fnlist [options] path...");
        }
    }

    if (optind == argc) display("-");
    while (optind < argc) {
        char *arg = strdup(argv[optind++]);
        if (!fix_path(arg, false)) {
            fprintf(stderr, "error: bad path: %s\n", arg);
            continue;
        }

        if (access(arg, R_OK)) {
            fprintf(stderr, "error: access: %s\n", arg);
            continue;
        }

        if (is_dir(arg))
            scan(arg);
        else
            display(arg);
    }
    return 0;
}
