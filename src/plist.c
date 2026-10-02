// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#include <local/scan.h>
#include <cpr/system.h>
#include <cpr/strchar.h>
#include <getopt.h>

static char end_quote[2] = {0, 0};
static char opt_quote[2] = {0, 0};
static char opt_delim[2] = {' ', 0};
static bool opt_escaped = false;
static bool opt_recursive = false;
static bool opt_directory = false;
static bool opt_native = false;
static bool opt_hidden = false;
static bool opt_basename = false;
static bool opt_lines = false;
static unsigned var_count = 0;
static bool var_newline = false;
static bool var_sublevel = false;
static bool var_leadin = false;

static bool is_hidden(const char *path) {
    if (!var_sublevel) return false;
    const char *cp = strrchr(path, '/');
    if (cp)
        ++cp;
    else
        cp = path;

    if (*cp == '.') return true;
    return false;
}

static void display(char *arg) {
    char *cp = arg;

#ifdef _WIN32
    while ((opt_basename || !opt_native) && NULL != (cp = strchr(cp, '\\')))
        *(cp++) = '/';
#endif

    cp = strrchr(arg, '/');
    if (opt_basename && cp) arg = ++cp;
    if (is_hidden(arg) && !opt_hidden) return;
    if (opt_escaped) {
        if (var_leadin)
            putchar(*opt_delim);
        else
            var_leadin = true;
        while (*arg) {
            unsigned char ch = *(arg++);
            switch (*arg) {
            case 8:
                putchar('\\');
                ch = 'b';
                break;
            case 9:
                putchar('\\');
                ch = 't';
                break;
            case 10:
                putchar('\\');
                ch = 'n';
                break;
            case 13:
                putchar('\\');
                ch = 'r';
                break;
            case 27:
                putchar('\\');
                ch = 'e';
                break;
            case ' ':
            case '\\':
                putchar('\\');
            default:
                break;
            }
            if (ch < 32 || ch >= 127)
                printf("\\%o", ch);
            else
                putchar(ch);
        }
    } else if (var_leadin && !opt_lines) {
        printf("%s%s%s%s", opt_delim, opt_quote, arg, end_quote);
    } else {
        var_leadin = true;
        printf("%s%s%s", opt_quote, arg, end_quote);
    }
    if (opt_lines)
        putchar('\n');
    else
        var_newline = true;
}

static void scan(const char *path) {
    char buf[512];
    if (is_hidden(path) && !opt_hidden) return;
    DIR *dir = opendir(path);
    struct dirent *entry;
    if (!dir) {
        perror("dir");
        exit(1);
    }
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.' && !opt_hidden) continue;
        snprintf(buf, sizeof(buf), "%s/%s", path, entry->d_name);
        display(buf);
        if (is_dir(buf) && opt_recursive) {
            var_sublevel = true;
            scan(buf);
        }
    }
    closedir(dir);
}

int main(int argc, char *argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "Rbdehlnq:s:")) != -1) {
        switch (opt) {
        case 'R':
            opt_directory = true;
            opt_recursive = true;
            break;
        case 'b':
            opt_basename = true;
            break;
        case 'e':
            opt_escaped = true;
            break;
        case 'h':
            opt_hidden = true;
            break;
        case 'd':
            opt_directory = true;
            break;
        case 'l':
            opt_lines = true;
            break;
        case 'n':
            opt_native = true;
            break;
        case 's':
            opt_delim[0] = scan_char(optarg);
            if (!opt_delim[0]) {
                fprintf(stderr, "%s: invalid delim: %s\n", argv[0], optarg);
                goto usage;
            }
            break;
        case 'q':
            opt_quote[0] = scan_quote(optarg);
            if (!opt_quote[0]) {
                fprintf(stderr, "%s: invalid quote: %s\n", argv[0], optarg);
                goto usage;
            }
            switch (opt_quote[0]) {
            case '(':
                end_quote[0] = ')';
                break;
            case ';':
                end_quote[0] = ']';
                break;
            case '{':
                end_quote[0] = '}';
                break;
            case '<':
                end_quote[0] = '>';
                break;
            default:
                end_quote[0] = opt_quote[0];
                break;
            }
            break;
        default:
        usage:
            cpr_crit(-1, "usage: plist [options] path...");
        }
    }
    while (optind < argc) {
        char *arg = argv[optind++];
        if (opt_directory && is_dir(arg))
            scan(arg);
        else
            display(arg);
    }
    if (var_newline) printf("\n");
    return 0;
}
