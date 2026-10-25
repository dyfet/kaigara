// SPDX-License-Identifier: BSD-3-Clause
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#include <local/scan.h>
#include <cpr/system.h>
#include <cpr/strchar.h>
#include <getopt.h>

static bool opt_hidden = false;
static bool opt_basename = false;

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
    char *cp = strrchr(arg, '/');
    if (opt_basename && cp) arg = ++cp;
    printf("%s\n", arg);
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
    while ((opt = getopt(argc, argv, "bh")) != -1) {
        switch (opt) {
        case 'b':
            opt_basename = true;
            break;
        case 'h':
            opt_hidden = true;
            break;
        default:
        usage:
            cpr_crit(-1, "usage: fnlist [options] path...");
        }
    }

    while (optind < argc) {
        char *arg = argv[optind++], *tail = "";

#ifdef _WIN32
        char *cp = arg;
        while ((opt_basename) && NULL != (cp = strchr(cp, '\\')))
            *(cp++) = '/';
#endif

        while (tail) {
            tail = strrchr(arg, '/');
            if (!tail || tail[1] != 0) break;
            *tail = 0;
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
